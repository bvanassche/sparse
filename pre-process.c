/*
 * Do C preprocessing, based on a token list gathered by
 * the tokenizer.
 *
 * This may not be the smartest preprocessor on the planet.
 *
 * Copyright (C) 2003 Transmeta Corp.
 *               2003-2004 Linus Torvalds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "lib.h"
#include "allocate.h"
#include "parse.h"
#include "token.h"
#include "symbol.h"
#include "expression.h"
#include "scope.h"

static struct ident_list *macros;	// only needed for -dD
static int false_nesting = 0;
static int counter_macro = 0;		// __COUNTER__ expansion
static int include_level = 0;
static int expanding = 0;
static int in_cond_expr = 0;

#define INCLUDEPATHS 300
const char *includepath[INCLUDEPATHS+1] = {
	"",
	"/usr/include",
	"/usr/local/include",
	NULL
};

static const char **quote_includepath = includepath;
static const char **angle_includepath = includepath + 1;
static const char **isys_includepath   = includepath + 1;
static const char **sys_includepath   = includepath + 1;
static const char **dirafter_includepath = includepath + 3;

#define dirty_stream(stream)				\
	do {						\
		if (!stream->dirty) {			\
			stream->dirty = 1;		\
			if (!stream->ifndef)		\
				stream->protect = NULL;	\
		}					\
	} while(0)

#define end_group(stream)					\
	do {							\
		if (stream->ifndef == stream->top_if) {		\
			stream->ifndef = NULL;			\
			if (!stream->dirty)			\
				stream->protect = NULL;		\
			else if (stream->protect)		\
				stream->dirty = 0;		\
		}						\
	} while(0)

#define nesting_error(stream)		\
	do {				\
		stream->dirty = 1;	\
		stream->ifndef = NULL;	\
		stream->protect = NULL;	\
	} while(0)

static struct token *alloc_token(struct position *pos)
{
	struct token *token = __alloc_token(0);

	token->pos.stream = pos->stream;
	token->pos.line = pos->line;
	token->pos.pos = pos->pos;
	token->pos.whitespace = 1;
	return token;
}

/* Expand symbol 'sym' at '*list' */
static int expand(struct token **, struct symbol *);

static void replace_with_string(struct token *token, const char *str)
{
	size_t size = strlen(str) + 1;
	struct string *s;

	if (size > INT_MAX - sizeof(struct string))
		die("string too large");
	s = __alloc_string(size);

	s->length = size;
	memcpy(s->data, str, size);
	token_type(token) = TOKEN_STRING;
	token->string = s;
}

static void replace_with_integer(struct token *token, unsigned int val)
{
	char *buf = __alloc_bytes(11);
	sprintf(buf, "%u", val);
	token_type(token) = TOKEN_NUMBER;
	token->number = buf;
}

static struct symbol *lookup_macro(struct ident *ident)
{
	struct symbol *sym = lookup_symbol(ident, NS_MACRO | NS_UNDEF);
	if (sym && !(sym->namespace & NS_MACRO))
		sym = NULL;
	return sym;
}

static int token_defined(struct token *token)
{
	if (token_type(token) == TOKEN_IDENT) {
		struct symbol *sym = lookup_macro(token->ident);
		if (sym) {
			sym->used_in = file_scope;
			return 1;
		}
		return 0;
	}

	sparse_error(token->pos, "expected preprocessor identifier");
	return 0;
}

static void replace_with_bool(struct token *token, bool val)
{
	static const char *string[] = { "0", "1" };

	token_type(token) = TOKEN_NUMBER;
	token->number = string[val];
}

static void replace_with_defined(struct token *token)
{
	replace_with_bool(token, token_defined(token));
}

static void expand_line(struct token *token)
{
	replace_with_integer(token, token->pos.line);
}

static void expand_file(struct token *token)
{
	replace_with_string(token, stream_name(token->pos.stream));
}

static void expand_basefile(struct token *token)
{
	replace_with_string(token, base_filename);
}

static time_t t = 0;
static void expand_date(struct token *token)
{
	static char buffer[12]; /* __DATE__: 3 + ' ' + 2 + ' ' + 4 + '\0' */

	if (!t)
		time(&t);
	strftime(buffer, 12, "%b %e %Y", localtime(&t));
	replace_with_string(token, buffer);
}

static void expand_time(struct token *token)
{
	static char buffer[9]; /* __TIME__: 2 + ':' + 2 + ':' + 2 + '\0' */

	if (!t)
		time(&t);
	strftime(buffer, 9, "%T", localtime(&t));
	replace_with_string(token, buffer);
}

static void expand_counter(struct token *token)
{
	replace_with_integer(token, counter_macro++);
}

static void expand_include_level(struct token *token)
{
	replace_with_integer(token, include_level - 1);
}

static int expand_one_symbol(struct token **list)
{
	struct token *token = *list;
	struct symbol *sym;

	if (token->pos.noexpand)
		return 1;

	sym = lookup_macro(token->ident);
	if (!sym)
		return 1;
	if (sym->expand_simple) {
		sym->expand_simple(token);
		return 1;
	} else {
		int rc;

		sym->used_in = file_scope;
		expanding = 1;
		rc = expand(list, sym);
		expanding = 0;
		return rc;
	}
}

static inline struct token *scan_next(struct token **where)
{
	struct token *token = *where;
	if (token_type(token) != TOKEN_UNTAINT)
		return token;
	do {
		token->ident->tainted = 0;
		token = token->next;
	} while (token_type(token) == TOKEN_UNTAINT);
	*where = token;
	return token;
}

static void expand_list(struct token **list)
{
	struct token *next;
	while (!eof_token(next = scan_next(list))) {
		if (token_type(next) != TOKEN_IDENT || expand_one_symbol(list))
			list = &next->next;
	}
}

static void preprocessor_line(struct stream *stream, struct token **line);

static struct token *collect_arg(struct token *prev, int vararg, const struct position *pos)
{
	struct stream *stream = input_streams + prev->pos.stream;
	struct token **p = &prev->next;
	struct token *next;
	int nesting = 0;

	while (!eof_token(next = scan_next(p))) {
		if (next->pos.newline && match_op(next, '#')) {
			if (!next->pos.noexpand) {
				preprocessor_line(stream, p);
				__free_token(next);	/* Free the '#' token */
				continue;
			}
		}
		switch (token_type(next)) {
		case TOKEN_STREAMEND:
		case TOKEN_STREAMBEGIN:
			*p = &eof_token_entry;
			return next;
		}
		if (false_nesting) {
			*p = next->next;
			__free_token(next);
			continue;
		}
		if (match_op(next, '(')) {
			nesting++;
		} else if (match_op(next, ')')) {
			if (!nesting--)
				break;
		} else if (match_op(next, ',') && !nesting && !vararg) {
			break;
		}
		next->pos.stream = pos->stream;
		next->pos.line = pos->line;
		next->pos.pos = pos->pos;
		next->pos.newline = 0;
		p = &next->next;
	}
	*p = &eof_token_entry;
	return next;
}

/*
 * We store arglist as <counter> [arg1] <number of uses for arg1> ... eof
 */

struct arg {
	struct token *arg;
	struct token *expanded;
	struct token *str;
	int n_normal;
	int n_quoted;
	int n_str;
};

static inline bool stream_end_token(const struct token *t)
{
	return eof_token(t) ||
	       token_type(t) == TOKEN_STREAMEND ||
	       token_type(t) == TOKEN_STREAMBEGIN;
}

static int collect_arguments(struct token *start, struct token *arglist, struct arg *args, struct token *what)
{
	int wanted = arglist->count.normal;
	struct token *next = NULL;
	int count = 0;

	arglist = arglist->next;	/* skip counter */

	if (!wanted) {
		next = collect_arg(start, 0, &what->pos);
		if (stream_end_token(next))
			goto Eclosing;
		if (!eof_token(start->next) || !match_op(next, ')')) {
			count++;
			goto Emany;
		}
	} else {
		for (count = 0; count < wanted; count++) {
			struct argcount *p = &arglist->next->count;
			next = collect_arg(start, p->vararg, &what->pos);
			if (stream_end_token(next))
				goto Eclosing;
			if (p->vararg && wanted == 1 && eof_token(start->next))
				break;
			arglist = arglist->next->next;
			args[count].arg = start->next;
			args[count].n_normal = p->normal;
			args[count].n_quoted = p->quoted;
			args[count].n_str = p->str;
			if (match_op(next, ')')) {
				count++;
				break;
			}
			start = next;
		}
		if (count == wanted && !match_op(next, ')'))
			goto Emany;
		if (count == wanted - 1) {
			struct argcount *p = &arglist->next->count;
			if (!p->vararg)
				goto Efew;
			args[count].arg = NULL;
			args[count].n_normal = p->normal;
			args[count].n_quoted = p->quoted;
			args[count].n_str = p->str;
		}
		if (count < wanted - 1)
			goto Efew;
	}
	what->next = next->next;
	return 1;

Efew:
	sparse_error(what->pos, "macro \"%s\" requires %d arguments, but only %d given",
		show_token(what), wanted, count);
	goto out;
Emany:
	while (match_op(next, ',')) {
		next = collect_arg(next, 0, &what->pos);
		count++;
	}
	if (stream_end_token(next))
		goto Eclosing;
	sparse_error(what->pos, "macro \"%s\" passed %d arguments, but takes just %d",
		show_token(what), count, wanted);
	goto out;
Eclosing:
	sparse_error(what->pos, "unterminated argument list invoking macro \"%s\"",
		show_token(what));
out:
	if (stream_end_token(next))
		what->next = next;
	else
		what->next = next->next;
	return 0;
}

static struct token *dup_list(struct token *list)
{
	struct token *res = NULL;
	struct token **p = &res;

	while (!eof_token(list)) {
		struct token *newtok = __alloc_token(0);
		*newtok = *list;
		*p = newtok;
		p = &newtok->next;
		list = list->next;
	}
	return res;
}

static const char *show_token_sequence(struct token *token, int quote)
{
	static char *buffer;
	static size_t buf_size;
	size_t offset = 0;
	size_t whitespace = 0;

	if (!token && !quote)
		return "<none>";
	while (!eof_token(token)) {
		const char *val = quote ? quote_token(token) : show_token(token);
		size_t len = strlen(val);
		size_t needed;

		if (len > SIZE_MAX - offset - whitespace - 1)
			die("out of memory");
		needed = offset + whitespace + len + 1;

		if (needed > buf_size) {
			size_t new_size = buf_size ? buf_size : 65536;
			while (new_size < needed) {
				if (new_size > SIZE_MAX / 2) {
					new_size = needed;
					break;
				}
				new_size *= 2;
			}
			buffer = realloc(buffer, new_size);
			if (!buffer)
				die("out of memory");
			buf_size = new_size;
		}

		if (whitespace)
			buffer[offset++] = ' ';
		memcpy(buffer + offset, val, len);
		offset += len;
		token = token->next;
		whitespace = token->pos.whitespace;
	}
	if (!buffer) {
		buf_size = 64;
		buffer = malloc(buf_size);
		if (!buffer)
			die("out of memory");
	}
	buffer[offset] = 0;
	return buffer;
}

static struct token *stringify(struct token *arg)
{
	const char *s = show_token_sequence(arg, 1);
	size_t size = strlen(s) + 1;
	struct token *token;
	struct string *string;

	if (size > INT_MAX - sizeof(struct string))
		die("string too large");
	token = __alloc_token(0);
	string = __alloc_string(size);

	memcpy(string->data, s, size);
	string->length = size;
	token->pos = arg->pos;
	token_type(token) = TOKEN_STRING;
	token->string = string;
	token->next = &eof_token_entry;
	return token;
}

static void expand_arguments(int count, struct arg *args)
{
	int i;
	for (i = 0; i < count; i++) {
		struct token *arg = args[i].arg;
		if (!arg)
			arg = &eof_token_entry;
		if (args[i].n_str)
			args[i].str = stringify(arg);
		if (args[i].n_normal) {
			if (!args[i].n_quoted) {
				args[i].expanded = arg;
				args[i].arg = NULL;
			} else if (eof_token(arg)) {
				args[i].expanded = arg;
			} else {
				args[i].expanded = dup_list(arg);
			}
			expand_list(&args[i].expanded);
		}
	}
}

/*
 * Possibly valid combinations:
 *  - ident + ident -> ident
 *  - ident + number -> ident unless number contains '.', '+' or '-'.
 *  - 'L' + char constant -> wide char constant
 *  - 'L' + string literal -> wide string literal
 *  - number + number -> number
 *  - number + ident -> number
 *  - number + '.' -> number
 *  - number + '+' or '-' -> number, if number used to end on [eEpP].
 *  - '.' + number -> number, if number used to start with a digit.
 *  - special + special -> either special or an error.
 */
static enum token_type combine(struct token *left, struct token *right, char *p)
{
	int len;
	enum token_type t1 = token_type(left), t2 = token_type(right);

	if (t1 != TOKEN_IDENT && t1 != TOKEN_NUMBER && t1 != TOKEN_SPECIAL)
		return TOKEN_ERROR;

	if (t1 == TOKEN_IDENT && left->ident == &L_ident) {
		if (t2 >= TOKEN_CHAR && t2 <= TOKEN_CHAR_EMBEDDED_3)
			return t2 + TOKEN_WIDE_CHAR - TOKEN_CHAR;
		if (t2 == TOKEN_STRING)
			return TOKEN_WIDE_STRING;
	}

	if (t1 == TOKEN_IDENT && left->ident == &u8_ident) {
		if (t2 >= TOKEN_CHAR && t2 <= TOKEN_CHAR_EMBEDDED_3)
			return t2 + TOKEN_UTF8_CHAR - TOKEN_CHAR;
		if (t2 == TOKEN_STRING)
			return TOKEN_UTF8_STRING;
	}

	if (t2 != TOKEN_IDENT && t2 != TOKEN_NUMBER && t2 != TOKEN_SPECIAL)
		return TOKEN_ERROR;

	{
		const char *s1 = show_token(left);
		int l1 = strlen(s1);
		const char *s2;
		int l2;

		if (l1 > 4096)
			return TOKEN_ERROR;
		memcpy(p, s1, l1);
		s2 = show_token(right);
		l2 = strlen(s2);
		if (l1 + l2 > 4096)
			return TOKEN_ERROR;
		memcpy(p + l1, s2, l2 + 1);
		len = l1 + l2;
	}

	if (t1 == TOKEN_IDENT) {
		if (t2 == TOKEN_SPECIAL)
			return TOKEN_ERROR;
		if (t2 == TOKEN_NUMBER && strpbrk(p, "+-."))
			return TOKEN_ERROR;
		return TOKEN_IDENT;
	}

	if (t1 == TOKEN_NUMBER) {
		if (t2 == TOKEN_SPECIAL) {
			switch (right->special) {
			case '.':
				break;
			case '+': case '-':
				if (strchr("eEpP", p[len - 2]))
					break;
			default:
				return TOKEN_ERROR;
			}
		}
		return TOKEN_NUMBER;
	}

	if (p[0] == '.' && isdigit((unsigned char)p[1]))
		return TOKEN_NUMBER;

	return TOKEN_SPECIAL;
}

static int merge(struct token *left, struct token *right)
{
	static char buffer[4097];
	enum token_type res = combine(left, right, buffer);
	int n;

	switch (res) {
	case TOKEN_IDENT:
		left->ident = built_in_ident(buffer);
		left->pos.noexpand = 0;
		return 1;

	case TOKEN_NUMBER:
		token_type(left) = TOKEN_NUMBER;	/* could be . + num */
		left->number = xstrdup(buffer);
		return 1;

	case TOKEN_SPECIAL:
		if (buffer[2] && buffer[3])
			break;
		for (n = SPECIAL_BASE; n < SPECIAL_ARG_SEPARATOR; n++) {
			if (!memcmp(buffer, combinations[n-SPECIAL_BASE], 3)) {
				left->special = n;
				return 1;
			}
		}
		break;

	case TOKEN_WIDE_CHAR:
	case TOKEN_WIDE_STRING:
	case TOKEN_UTF8_CHAR:
	case TOKEN_UTF8_STRING:
		token_type(left) = res;
		left->pos.noexpand = 0;
		left->string = right->string;
		return 1;

	case TOKEN_WIDE_CHAR_EMBEDDED_0 ... TOKEN_WIDE_CHAR_EMBEDDED_3:
	case TOKEN_UTF8_CHAR_EMBEDDED_0 ... TOKEN_UTF8_CHAR_EMBEDDED_3:
		token_type(left) = res;
		left->pos.noexpand = 0;
		memcpy(left->embedded, right->embedded, 4);
		return 1;

	default:
		;
	}
	sparse_error(left->pos, "'##' failed: concatenation is not a valid token");
	return 0;
}

static struct token *dup_token(const struct token *token, struct position *streampos)
{
	struct position pos = *streampos;
	struct token *alloc = __alloc_token(0);

	alloc->pos = token->pos;
	alloc->number = token->number;
	alloc->pos.stream = pos.stream;
	alloc->pos.line = pos.line;
	alloc->pos.pos = pos.pos;
	if (token_type(alloc) == TOKEN_STRING ||
	    token_type(alloc) == TOKEN_WIDE_STRING ||
	    token_type(alloc) == TOKEN_UTF8_STRING)
		token->string->immutable = 1;
	return alloc;	
}

static struct token **copy(struct token **where, struct token *list, int *count)
{
	int need_copy = --*count;
	do {
		struct token *token;
		if (need_copy)
			token = dup_token(list, &list->pos);
		else
			token = list;
		if (token_type(token) == TOKEN_IDENT && token->ident->tainted)
			token->pos.noexpand = 1;
		*where = token;
		where = &token->next;
		list = list->next;
	} while (!eof_token(list));
	*where = &eof_token_entry;
	return where;
}

static int handle_kludge(const struct token **p, struct arg *args)
{
	const struct token *t = (*p)->next->next;
	while (1) {
		struct arg *v = &args[t->argnum];
		if (token_type(t->next) != TOKEN_CONCAT) {
			if (v->arg) {
				/* ignore the first ## */
				*p = (*p)->next;
				return 0;
			}
			/* skip the entire thing */
			*p = t;
			return 1;
		}
		if (v->arg && !eof_token(v->arg))
			return 0; /* no magic */
		t = t->next->next;
	}
}

static inline int arg_empty(struct arg *arg)
{
	struct token *t = arg->expanded;

	if (!t)
		return 1;
	while (token_type(t) == TOKEN_UNTAINT)
		t = t->next;
	return eof_token(t);
}

static struct token **substitute_tokens(struct token **list,
					struct position *base_pos,
					const struct token *body,
					struct arg *args, int vararg_num)
{
	int *count;
	int one;
	enum {Normal, Placeholder, Concat} state = Normal;
	int extra_whitespace = 0;

	for (; !eof_token(body); body = body->next) {
		struct token *added, *arg;
		struct token **tail;
		const struct token *t;

		switch (token_type(body)) {
		case TOKEN_GNU_KLUDGE:
			/*
			 * GNU kludge: if we had <comma>##<vararg>, behaviour
			 * depends on whether we had enough arguments to have
			 * a vararg.  If we did, ## is just ignored.  Otherwise
			 * both , and ## are ignored.  Worse, there can be
			 * an arbitrary number of ##<arg> in between; if all of
			 * those are empty, we act as if they hadn't been there,
			 * otherwise we act as if the kludge didn't exist.
			 */
			t = body;
			if (handle_kludge(&body, args)) {
				if (state == Concat)
					state = Normal;
				else
					state = Placeholder;
				continue;
			}
			added = dup_token(t, base_pos);
			token_type(added) = TOKEN_SPECIAL;
			tail = &added->next;
			break;

		case TOKEN_STR_VA_OPT: {
			struct token *inner = &eof_token_entry;

			if (vararg_num >= 0 && !arg_empty(&args[vararg_num]))
				substitute_tokens(&inner, base_pos,
						  body->target, args,
						  vararg_num);
			added = stringify(inner);
			added->pos.stream = base_pos->stream;
			added->pos.line = base_pos->line;
			added->pos.pos = base_pos->pos;
			added->pos.newline = body->pos.newline;
			added->pos.whitespace =
				body->pos.whitespace || extra_whitespace;
			extra_whitespace = 0;
			tail = &added->next;
			break;
		}

		case TOKEN_VA_OPT: {
			struct token *inner = &eof_token_entry;

			if (vararg_num >= 0 && !arg_empty(&args[vararg_num]))
				substitute_tokens(&inner, base_pos,
						  body->target, args,
						  vararg_num);
			if (eof_token(inner)) {
				if (body->pos.whitespace && state != Concat &&
				    token_type(body->next) != TOKEN_CONCAT)
					extra_whitespace = 1;
				if (state == Concat)
					state = Normal;
				else if (token_type(body->next) == TOKEN_CONCAT)
					state = Placeholder;
				else
					state = Normal;
				continue;
			}
			arg = inner;
			one = 1;
			count = &one;
			goto copy_arg;
		}

		case TOKEN_STR_ARGUMENT:
			arg = args[body->argnum].str;
			count = &args[body->argnum].n_str;
			if (eof_token(arg)) {
				state = Normal;
				continue;
			}
			goto copy_arg;

		case TOKEN_QUOTED_ARGUMENT:
			arg = args[body->argnum].arg;
			count = &args[body->argnum].n_quoted;
			if (!arg || eof_token(arg)) {
				if (state == Concat)
					state = Normal;
				else
					state = Placeholder;
				continue;
			}
			goto copy_arg;

		case TOKEN_MACRO_ARGUMENT:
			arg = args[body->argnum].expanded;
			count = &args[body->argnum].n_normal;
			if (eof_token(arg)) {
				state = Normal;
				continue;
			}
		copy_arg:
			tail = copy(&added, arg, count);
			added->pos.newline = body->pos.newline;
			added->pos.whitespace = body->pos.whitespace;
			if (extra_whitespace) {
				added->pos.whitespace = 1;
				extra_whitespace = 0;
			}
			break;

		case TOKEN_CONCAT:
			if (state == Placeholder)
				state = Normal;
			else
				state = Concat;
			continue;

		default:
			added = dup_token(body, base_pos);
			if (token_type(body) == TOKEN_IDENT &&
			    added->ident->tainted)
				added->pos.noexpand = 1;
			if (extra_whitespace) {
				added->pos.whitespace = 1;
				extra_whitespace = 0;
			}
			tail = &added->next;
			break;
		}

		/*
		 * if we got to doing real concatenation, we already have
		 * added something into the list, so containing_token() is OK.
		 */
		if (state == Concat && merge(containing_token(list), added)) {
			*list = added->next;
			if (tail != &added->next)
				list = tail;
		} else {
			*list = added;
			list = tail;
		}
		state = Normal;
	}
	*list = &eof_token_entry;
	return list;
}

static struct token **substitute(struct token **list, const struct token *body,
				 struct arg *args, int vararg_num)
{
	struct position base_pos = (*list)->pos;

	return substitute_tokens(list, &base_pos, body, args, vararg_num);
}

static bool parse_has_include(struct token **list, int how);
static bool parse_has_embed(struct token **list);

static void reject_outside_directive(struct token *token)
{
	struct token *next;

	sparse_error(token->pos, "'%s' used outside of preprocessing directive",
		     show_token(token));
	next = scan_next(&token->next);
	if (match_op(next, '(')) {
		int depth = 1;

		next = next->next;
		while (!eof_token(next) && depth > 0) {
			if (match_op(next, '('))
				depth++;
			else if (match_op(next, ')'))
				depth--;
			next = next->next;
		}
	}
	token_type(token) = TOKEN_NUMBER;
	token->number = "0";
	token->next = next;
}

static int expand(struct token **list, struct symbol *sym)
{
	struct token *last;
	struct token *token = *list;
	struct ident *expanding = token->ident;
	struct token **tail;
	struct token *expansion = sym->expansion;
	int nargs = sym->arglist ? sym->arglist->count.normal : 0;
	int vararg_num = (sym->arglist && sym->arglist->count.vararg) ?
				 (sym->arglist->count.normal - 1) :
				 -1;
	struct arg args[nargs ? nargs : 1];

	if (nargs)
		memset(args, 0, nargs * sizeof(args[0]));

	if (expanding->tainted) {
		token->pos.noexpand = 1;
		return 1;
	}

	if (expanding == &__has_include_ident ||
	    expanding == &__has_include_next_ident ||
	    expanding == &__has_embed_ident) {
		bool ok;

		if (!in_cond_expr) {
			reject_outside_directive(token);
			return 0;
		}
		if (expanding == &__has_include_ident)
			ok = parse_has_include(list, 0);
		else if (expanding == &__has_include_next_ident)
			ok = parse_has_include(list, 1);
		else
			ok = parse_has_embed(list);
		if (!ok)
			replace_with_bool(token, 0);
		return 0;
	}

	if (sym->arglist) {
		if (!match_op(scan_next(&token->next), '('))
			return 1;
		if (!collect_arguments(token->next, sym->arglist, args, token))
			return 1;
		expand_arguments(nargs, args);
	}

	if (sym->expand)
		return sym->expand(token, args) ? 0 : 1;

	if (!expansion)
		return 1;

	expanding->tainted = 1;

	last = token->next;
	tail = substitute(list, expansion, args, vararg_num);
	/*
	 * Note that it won't be eof - at least TOKEN_UNTAINT will be there.
	 * We still can lose the newline flag if the sucker expands to nothing,
	 * but the price of dealing with that is probably too high (we'd need
	 * to collect the flags during scan_next())
	 */
	(*list)->pos.newline = token->pos.newline;
	(*list)->pos.whitespace = token->pos.whitespace;
	*tail = last;

	return 0;
}

static const char *token_name_sequence(struct token *token, int endop, struct token *start)
{
	static char buffer[PATH_MAX];
	char *ptr = buffer;

	while (!eof_token(token) && !match_op(token, endop)) {
		size_t len, rem;
		const char *val;
		if (token_type(token) == TOKEN_STRING)
			val = token->string->data;
		else
			val = show_token(token);
		len = strlen(val);
		rem = sizeof(buffer) - (size_t)(ptr - buffer);
		if (len >= rem) {
			sparse_error(token->pos, "header name too long");
			return NULL;
		}
		memcpy(ptr, val, len);
		ptr += len;
		token = token->next;
	}
	*ptr = 0;
	if (endop && !match_op(token, endop))
		sparse_error(start->pos, "expected '>' at end of filename");
	return buffer;
}

static int already_tokenized(const char *path)
{
	int stream, next;

	for (stream = *hash_stream(path); stream >= 0 ; stream = next) {
		struct stream *s = input_streams + stream;

		next = s->next_stream;
		if (s->once) {
			if (strcmp(path, s->name))
				continue;
			return 1;
		}
		if (s->constant != CONSTANT_FILE_YES)
			continue;
		if (strcmp(path, s->name))
			continue;
		if (s->protect && !lookup_macro(s->protect))
			continue;
		return 1;
	}
	return 0;
}

/* Handle include of header files.
 * The relevant options are made compatible with gcc. The only options that
 * are not supported is -withprefix and friends.
 *
 * Three set of include paths are known:
 * quote_includepath:	Path to search when using #include "file.h"
 * angle_includepath:	Paths to search when using #include <file.h>
 * isys_includepath:	Paths specified with -isystem, come before the
 *			built-in system include paths. Gcc would suppress
 *			warnings from system headers. Here we separate
 *			them from the angle_ ones to keep search ordering.
 *
 * sys_includepath:	Built-in include paths.
 * dirafter_includepath Paths added with -dirafter.
 *
 * The above is implemented as one array with pointers
 *                         +--------------+
 * quote_includepath --->  |              |
 *                         +--------------+
 *                         |              |
 *                         +--------------+
 * angle_includepath --->  |              |
 *                         +--------------+
 * isys_includepath  --->  |              |
 *                         +--------------+
 * sys_includepath   --->  |              |
 *                         +--------------+
 * dirafter_includepath -> |              |
 *                         +--------------+
 *
 * -I dir insert dir just before isys_includepath and move the rest
 * -I- makes all dirs specified with -I before to quote dirs only and
 *   angle_includepath is set equal to isys_includepath.
 * -nostdinc removes all sys dirs by storing NULL in entry pointed
 *   to by * sys_includepath. Note that this will reset all dirs built-in
 *   and added before -nostdinc by -isystem and -idirafter.
 * -isystem dir adds dir where isys_includepath points adding this dir as
 *   first systemdir
 * -idirafter dir adds dir to the end of the list
 */

static void set_stream_include_path(struct stream *stream)
{
	const char *path = stream->path;
	if (!path) {
		const char *p = strrchr(stream->name, '/');
		path = "";
		if (p) {
			int len = p - stream->name + 1;
			char *m = malloc(len+1);
			/* This includes the final "/" */
			memcpy(m, stream->name, len);
			m[len] = 0;
			path = m;
			/* normalize this path */
			while (path[0] == '.' && path[1] == '/') {
				path += 2;
				while (path[0] == '/')
					path++;
			}
		}
		stream->path = path;
	}
	includepath[0] = path;
}

static int try_include(struct position pos, const char *path, const char *filename, int flen, struct token **where, const char **next_path)
{
	int fd;
	struct stat st;
	int plen = strlen(path);
	char fullname[PATH_MAX];

	if (!flen || plen + flen + 2 > sizeof(fullname))
		return 0;
	memcpy(fullname, path, plen);
	if (plen && path[plen-1] != '/') {
		fullname[plen] = '/';
		plen++;
	}
	memcpy(fullname+plen, filename, flen);
	fullname[plen + flen] = '\0';
	if (already_tokenized(fullname))
		return 1;
	fd = open(fullname, O_RDONLY);
	if (fd >= 0) {
		if (fstat(fd, &st) == 0 && !S_ISDIR(st.st_mode)) {
			char *streamname = xmemdup(fullname, plen + flen + 1);
			*where = tokenize(&pos, streamname, fd, *where, next_path);
			close(fd);
			return 1;
		}
		close(fd);
	}
	return 0;
}

static int do_include_path(const char **pptr, struct token **list, struct token *token, const char *filename, int flen)
{
	const char *path;

	while ((path = *pptr++) != NULL) {
		if (!try_include(token->pos, path, filename, flen, list, pptr))
			continue;
		return 1;
	}
	return 0;
}

static int test_include(const char *path, const char *filename)
{
	int fd;
	struct stat st;
	int plen = strlen(path);
	int flen = strlen(filename);
	char fullname[PATH_MAX];

	if (plen + flen + 2 > sizeof(fullname))
		return 0;
	memcpy(fullname, path, plen);
	if (plen && path[plen - 1] != '/') {
		fullname[plen] = '/';
		plen++;
	}
	memcpy(fullname + plen, filename, flen + 1);
	if (already_tokenized(fullname))
		return 1;
	fd = open(fullname, O_RDONLY);
	if (fd >= 0) {
		if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode)) {
			close(fd);
			return 1;
		}
		close(fd);
	}
	return 0;
}

static int check_include(const char **pptr, const char *filename)
{
	const char *path;
	while ((path = *pptr++) != NULL) {
		if (test_include(path, filename))
			return 1;
	}
	return 0;
}

static int check_has_include(struct stream *stream, const char *filename,
			     int expect, int how)
{
	const char **path;

	/* Absolute path? */
	if (filename[0] == '/')
		return test_include("", filename);

	switch (how) {
	case 1:
		path = stream->next_path;
		break;
	default:
		set_stream_include_path(stream);
		path = expect ? angle_includepath : quote_includepath;
		break;
	}
	if (!path)
		return 0;
	return check_include(path, filename);
}

static struct token *skip_balanced_parens(struct token *lparen)
{
	struct token *t;
	int depth = 1;

	if (!match_op(lparen, '('))
		return lparen;
	t = lparen->next;
	while (!eof_token(t) && depth > 0) {
		if (match_op(t, '('))
			depth++;
		else if (match_op(t, ')'))
			depth--;
		t = t->next;
	}
	return t;
}

static bool collect_angle_filename(struct token **next_p, char *buf,
				   size_t size, struct position pos)
{
	struct token *t = *next_p;
	char *ptr = buf;

	while (!eof_token(t = scan_next(&t)) && !match_op(t, '>')) {
		const char *s = (token_type(t) == TOKEN_STRING) ?
					t->string->data :
					show_token(t);
		size_t len = strlen(s);
		size_t rem = size - (size_t)(ptr - buf);

		if (len >= rem) {
			sparse_error(t->pos, "header name too long");
			return false;
		}
		memcpy(ptr, s, len);
		ptr += len;
		t = t->next;
	}
	*ptr = '\0';
	if (!match_op(t, '>')) {
		sparse_error(pos, "expected '>'");
		return false;
	}
	if (ptr == buf) {
		sparse_error(pos, "empty filename");
		return false;
	}
	*next_p = scan_next(&t->next);
	return true;
}

static bool extract_string_filename(struct token **next_p, char *buf,
				    size_t size)
{
	struct token *t = *next_p;
	const char *s = t->string->data;
	size_t len = strlen(s);

	if (!len) {
		sparse_error(t->pos, "empty filename");
		return false;
	}
	if (len >= size) {
		sparse_error(t->pos, "header name too long");
		return false;
	}
	memcpy(buf, s, len + 1);
	*next_p = t->next;
	return true;
}

static bool parse_has_include(struct token **list, int how)
{
	struct token *token = *list;
	struct token *lparen = scan_next(&token->next);
	struct token *next;
	struct stream *stream = input_streams + token->pos.stream;
	char filename[PATH_MAX];
	int expect = 0;
	int val = 0;

	if (!match_op(lparen, '(')) {
		sparse_error(token->pos, "expected '(' after %s",
			     show_token(token));
		return false;
	}
	next = scan_next(&lparen->next);
	if (match_op(next, '<')) {
		expect = '>';
		next = next->next;
		if (!collect_angle_filename(&next, filename, sizeof(filename),
					    token->pos))
			goto err;
	} else if (token_type(next) == TOKEN_STRING) {
		if (!extract_string_filename(&next, filename, sizeof(filename)))
			goto err;
	} else {
		struct token **p = &lparen->next;
		struct token *rparen = scan_next(p);
		int depth = 1;

		while (!eof_token(rparen)) {
			if (match_op(rparen, '('))
				depth++;
			else if (match_op(rparen, ')') && !--depth)
				break;
			p = &rparen->next;
			rparen = scan_next(p);
		}
		if (!match_op(rparen, ')')) {
			sparse_error(rparen->pos, "expected ')'");
			token->next = rparen;
			return false;
		}
		*p = &eof_token_entry;
		expand_list(&lparen->next);
		p = &lparen->next;
		while (!eof_token(*p))
			p = &(*p)->next;
		*p = rparen;
		next = lparen->next;
		if (match_op(next, '<')) {
			expect = '>';
			next = next->next;
			if (!collect_angle_filename(&next, filename,
						    sizeof(filename),
						    token->pos))
				goto err;
		} else if (token_type(next) == TOKEN_STRING) {
			if (!extract_string_filename(&next, filename,
						     sizeof(filename)))
				goto err;
		} else {
			sparse_error(next->pos,
				     "expected \"FILENAME\" or <FILENAME>");
			goto err;
		}
	}
	next = scan_next(&next);
	if (!match_op(next, ')')) {
		sparse_error(next->pos, "expected ')'");
		goto err;
	}
	next = next->next;

	val = check_has_include(stream, filename, expect, how);
	replace_with_bool(token, val);
	token->next = next;
	return true;

err:
	token->next = skip_balanced_parens(lparen);
	return false;
}

static void replace_with_int(struct token *token, int val)
{
	static const char *const digits[] = { "0", "1", "2" };
	token_type(token) = TOKEN_NUMBER;
	token->number = (val >= 0 && val <= 2) ? digits[val] : "0";
}

#define MAX_EMBED_SIZE (16 * 1024 * 1024)

enum embed_param_kind {
	EMBED_PARAM_NONE = 0,
	EMBED_PARAM_LIMIT = 1 << 0,
	EMBED_PARAM_PREFIX = 1 << 1,
	EMBED_PARAM_SUFFIX = 1 << 2,
	EMBED_PARAM_IF_EMPTY = 1 << 3,
};

static enum embed_param_kind classify_embed_param(struct ident *id)
{
	if (id == &limit_ident || id == &__limit___ident)
		return EMBED_PARAM_LIMIT;
	if (id == &prefix_ident || id == &__prefix___ident)
		return EMBED_PARAM_PREFIX;
	if (id == &suffix_ident || id == &__suffix___ident)
		return EMBED_PARAM_SUFFIX;
	if (id == &if_empty_ident || id == &__if_empty___ident)
		return EMBED_PARAM_IF_EMPTY;
	return EMBED_PARAM_NONE;
}

static struct token *dup_token_range(struct token *start, struct token *end)
{
	struct token *head = &eof_token_entry;
	struct token **tail = &head;
	struct token *p;

	if (!start || !end)
		return &eof_token_entry;
	for (p = start;; p = p->next) {
		struct token *t = __alloc_token(0);
		*t = *p;
		*tail = t;
		tail = &t->next;
		if (p == end)
			break;
	}
	*tail = &eof_token_entry;
	return head;
}

static bool eval_embed_limit_expr(struct position pos, struct token *pstart,
				  struct token *pend,
				  unsigned long long *limit_val)
{
	struct token *copy = dup_token_range(pstart, pend);
	struct expression *expr = NULL;
	long long sval;

	if (eof_token(copy)) {
		sparse_error(pos, "expected expression in #embed limit");
		return false;
	}
	in_cond_expr++;
	expand_list(&copy);
	for (struct token *t = copy; !eof_token(t); t = t->next) {
		if (token_type(t) != TOKEN_IDENT)
			continue;
		if (standard >= STANDARD_C23 && t->ident == &true_ident) {
			replace_with_bool(t, 1);
		} else if (standard >= STANDARD_C23 &&
			   t->ident == &false_ident) {
			replace_with_bool(t, 0);
		} else if (t->ident == &defined_ident) {
			in_cond_expr--;
			sparse_error(t->pos,
				     "'defined' cannot appear in #embed limit");
			return false;
		} else {
			token_type(t) = TOKEN_ZERO_IDENT;
		}
	}
	in_cond_expr--;
	copy = constant_expression(copy, &expr);
	if (!expr || !eof_token(copy)) {
		sparse_error(pos, "invalid #embed limit");
		return false;
	}
	sval = get_expression_value(expr);
	if (!expr->ctype || expr->type != EXPR_VALUE)
		return false;
	if (sval < 0 && !(expr->ctype->ctype.modifiers & MOD_UNSIGNED)) {
		sparse_error(expr->pos, "negative #embed limit");
		return false;
	}
	*limit_val = (unsigned long long)sval;
	return true;
}

static int test_embed_file(const char *path, const char *filename,
			   int has_limit, unsigned long long limit_val,
			   unsigned long long *size)
{
	int fd;
	struct stat st;
	size_t plen = strlen(path);
	size_t flen = strlen(filename);
	char fullname[PATH_MAX];

	if (plen + flen + 2 > sizeof(fullname))
		return 0;
	memcpy(fullname, path, plen);
	if (plen && path[plen - 1] != '/') {
		fullname[plen] = '/';
		plen++;
	}
	memcpy(fullname + plen, filename, flen + 1);
	fd = open(fullname, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
		return 0;
	if (fstat(fd, &st) < 0 || S_ISDIR(st.st_mode)) {
		close(fd);
		return 0;
	}
	if (S_ISREG(st.st_mode)) {
		unsigned long long eff = st.st_size;

		close(fd);
		if (has_limit && limit_val < eff)
			eff = limit_val;
		if (eff > MAX_EMBED_SIZE)
			return 0;
		*size = eff;
		return 1;
	}
	if (S_ISCHR(st.st_mode)) {
		unsigned char b;
		ssize_t r;

		if (has_limit && limit_val == 0) {
			close(fd);
			*size = 0;
			return 1;
		}
		if (has_limit && limit_val > MAX_EMBED_SIZE) {
			close(fd);
			return 0;
		}
		r = read(fd, &b, 1);
		close(fd);
		if (r == 0) {
			*size = 0;
			return 1;
		}
		if (r > 0 && has_limit) {
			*size = 1;
			return 1;
		}
		return 0;
	}
	close(fd);
	return 0;
}

static int check_has_embed(struct stream *stream, const char *filename,
			   int expect, int has_limit,
			   unsigned long long limit_val,
			   unsigned long long *size)
{
	const char **path;

	/* Absolute path? */
	if (filename[0] == '/')
		return test_embed_file("", filename, has_limit, limit_val,
				       size);

	set_stream_include_path(stream);
	path = expect ? angle_includepath : quote_includepath;
	if (!path)
		return 0;
	while (*path != NULL) {
		if (test_embed_file(*path++, filename, has_limit, limit_val,
				    size))
			return 1;
	}
	return 0;
}

static bool parse_has_embed(struct token **list)
{
	struct token *token = *list;
	struct token *lparen = scan_next(&token->next);
	struct token *next;
	struct token *params, **param_p, *rparen, *after_rparen;
	struct stream *stream = input_streams + token->pos.stream;
	char filename[PATH_MAX];
	int expect = 0;
	int val = 0;
	unsigned long long filesize = 0;
	int has_limit = 0;
	unsigned long long limit_val = ~0ULL;
	unsigned int seen_params = 0;
	int supported = 1;

	if (!match_op(lparen, '(')) {
		sparse_error(token->pos, "expected '(' after %s",
			     show_token(token));
		return false;
	}
	next = scan_next(&lparen->next);
	if (match_op(next, '<')) {
		expect = '>';
		next = next->next;
		if (!collect_angle_filename(&next, filename, sizeof(filename),
					    token->pos))
			goto err;
	} else if (token_type(next) == TOKEN_STRING) {
		if (!extract_string_filename(&next, filename, sizeof(filename)))
			goto err;
	} else {
		struct token **p = &lparen->next;
		int depth = 1;

		rparen = scan_next(p);
		while (!eof_token(rparen)) {
			if (match_op(rparen, '('))
				depth++;
			else if (match_op(rparen, ')') && !--depth)
				break;
			p = &rparen->next;
			rparen = scan_next(p);
		}
		if (!match_op(rparen, ')')) {
			sparse_error(rparen->pos, "expected ')'");
			token->next = rparen;
			return false;
		}
		*p = &eof_token_entry;
		expand_list(&lparen->next);
		p = &lparen->next;
		while (!eof_token(*p))
			p = &(*p)->next;
		*p = rparen;
		next = lparen->next;
		if (match_op(next, '<')) {
			expect = '>';
			next = next->next;
			if (!collect_angle_filename(&next, filename,
						    sizeof(filename),
						    token->pos))
				goto err;
		} else if (token_type(next) == TOKEN_STRING) {
			if (!extract_string_filename(&next, filename,
						     sizeof(filename)))
				goto err;
		} else {
			sparse_error(next->pos,
				     "expected \"FILENAME\" or <FILENAME>");
			goto err;
		}
	}

	/* Isolate optional parameters before matching ')' */
	params = next;
	{
		struct token **p = &params;
		int depth = 1;

		rparen = scan_next(p);
		while (!eof_token(rparen)) {
			if (match_op(rparen, '('))
				depth++;
			else if (match_op(rparen, ')') && !--depth)
				break;
			p = &rparen->next;
			rparen = scan_next(p);
		}
		if (!match_op(rparen, ')')) {
			sparse_error(rparen->pos, "expected ')'");
			token->next = rparen;
			return false;
		}
		after_rparen = rparen->next;
		*p = &eof_token_entry;
	}

	param_p = &params;
	while (!eof_token(next = scan_next(param_p))) {
		if (token_type(next) == TOKEN_IDENT) {
			struct ident *id;
			struct position id_pos;
			enum embed_param_kind kind;

			if (!expand_one_symbol(param_p))
				continue;
			next = *param_p;
			if (token_type(next) != TOKEN_IDENT) {
				supported = 0;
				param_p = &next->next;
				continue;
			}
			id = next->ident;
			id_pos = next->pos;
			if (match_op(next->next, ':') &&
			    match_op(next->next->next, ':')) {
				struct token *attr = next->next->next->next;

				if (token_type(attr) != TOKEN_IDENT) {
					sparse_error(attr->pos,
						     "identifier expected");
					token->next = after_rparen;
					return false;
				}
				supported = 0;
				next = attr->next;
				if (match_op(next, '(')) {
					int depth = 1;

					next = next->next;
					while (!eof_token(next) && depth > 0) {
						if (match_op(next, '('))
							depth++;
						else if (match_op(next, ')'))
							depth--;
						next = next->next;
					}
				}
				param_p = &next;
				continue;
			}

			kind = classify_embed_param(id);
			if (kind != EMBED_PARAM_NONE) {
				struct token *pstart, *pend = NULL;
				int depth = 1;

				if (seen_params & kind) {
					sparse_error(
						id_pos,
						"duplicate embed parameter '%s'",
						show_ident(id));
					token->next = after_rparen;
					return false;
				}
				seen_params |= kind;

				next = scan_next(&next->next);
				if (!match_op(next, '(')) {
					sparse_error(
						id_pos,
						"expected '(' after '%s'",
						show_ident(id));
					token->next = after_rparen;
					return false;
				}
				pstart = next->next;
				next = next->next;
				while (!eof_token(next) && depth > 0) {
					if (match_op(next, '('))
						depth++;
					else if (match_op(next, ')') &&
						 !--depth)
						break;
					pend = next;
					next = next->next;
				}
				if (!match_op(next, ')')) {
					sparse_error(id_pos, "expected ')'");
					token->next = after_rparen;
					return false;
				}
				next = next->next;
				if (kind == EMBED_PARAM_LIMIT) {
					if (!eval_embed_limit_expr(
						    id_pos, pstart, pend,
						    &limit_val)) {
						token->next = after_rparen;
						return false;
					}
					has_limit = 1;
				}
				param_p = &next;
				continue;
			}

			/* Unknown non-prefixed parameter */
			supported = 0;
			next = scan_next(&next->next);
			if (match_op(next, '(')) {
				int depth = 1;

				next = next->next;
				while (!eof_token(next) && depth > 0) {
					if (match_op(next, '('))
						depth++;
					else if (match_op(next, ')'))
						depth--;
					next = next->next;
				}
			}
			param_p = &next;
			continue;
		}
		supported = 0;
		param_p = &next->next;
	}

	if (!supported) {
		val = 0;
	} else if (!check_has_embed(stream, filename, expect, has_limit,
				    limit_val, &filesize)) {
		val = 0;
	} else if (filesize == 0 || (has_limit && limit_val == 0)) {
		val = 2;
	} else {
		val = 1;
	}

	replace_with_int(token, val);
	token->next = after_rparen;
	return true;

err:
	token->next = skip_balanced_parens(lparen);
	return false;
}

static int open_embed_file(struct stream *stream, const char *filename,
			   int expect)
{
	char fullname[PATH_MAX];
	const char **path;
	size_t flen = strlen(filename);

	if (filename[0] == '/')
		return open(filename, O_RDONLY);

	set_stream_include_path(stream);
	path = expect ? angle_includepath : quote_includepath;
	if (!path)
		return -1;
	while (*path != NULL) {
		const char *p = *path++;
		size_t plen = strlen(p);
		if (plen + flen + 2 > sizeof(fullname))
			continue;
		memcpy(fullname, p, plen);
		if (plen && p[plen - 1] != '/') {
			fullname[plen] = '/';
			plen++;
		}
		memcpy(fullname + plen, filename, flen + 1);
		int fd = open(fullname, O_RDONLY);
		if (fd >= 0)
			return fd;
	}
	return -1;
}

static int free_preprocessor_line(struct token *token);

static int handle_embed(struct stream *stream, struct token **line,
			struct token *token)
{
	char filename[PATH_MAX];
	int expect = 0;
	struct token *next = token->next;
	struct token **param_p;
	struct token *prefix_start = NULL, *prefix_end = NULL;
	struct token *suffix_start = NULL, *suffix_end = NULL;
	struct token *if_empty_start = NULL, *if_empty_end = NULL;
	int has_limit = 0;
	unsigned long long limit_val = ~0ULL;
	unsigned int seen_params = 0;
	int fd;
	struct stat st;
	unsigned char *buf = NULL;
	ssize_t bytes_read = 0;
	struct token *head = NULL, **tail = &head;
	static char num_strs[256][4];
	static int num_strs_inited = 0;

	if (standard < STANDARD_C23 && Wpedantic)
		warning(token->pos, "#embed before C23 is an extension");

	next = scan_next(&token->next);
	if (match_op(next, '<')) {
		expect = '>';
		next = next->next;
		if (!collect_angle_filename(&next, filename, sizeof(filename),
					    token->pos))
			return 1;
	} else if (token_type(next) == TOKEN_STRING) {
		if (!extract_string_filename(&next, filename, sizeof(filename)))
			return 1;
	} else {
		while (token_type(next = scan_next(&token->next)) ==
			       TOKEN_IDENT &&
		       !expand_one_symbol(&token->next))
			;
		next = scan_next(&token->next);
		if (match_op(next, '<')) {
			struct token **p = &next->next;
			struct token *t;

			while (!eof_token(t = scan_next(p)) &&
			       !match_op(t, '>')) {
				if (token_type(t) != TOKEN_IDENT ||
				    expand_one_symbol(p))
					p = &t->next;
			}
			expect = '>';
			next = next->next;
			if (!collect_angle_filename(&next, filename,
						    sizeof(filename),
						    token->pos))
				return 1;
		} else if (token_type(next) == TOKEN_STRING) {
			if (!extract_string_filename(&next, filename,
						     sizeof(filename)))
				return 1;
		} else {
			sparse_error(next->pos,
				     "expected \"FILENAME\" or <FILENAME>");
			return 1;
		}
	}

	param_p = &next;
	while (!eof_token(next = scan_next(param_p))) {
		if (token_type(next) == TOKEN_IDENT) {
			struct ident *id;
			struct position id_pos;
			enum embed_param_kind kind;
			struct token *pstart, *pend = NULL;
			int depth = 1;

			if (!expand_one_symbol(param_p))
				continue;
			next = *param_p;
			if (token_type(next) != TOKEN_IDENT) {
				sparse_error(next->pos,
					     "unexpected token in #embed");
				return 1;
			}
			id = next->ident;
			id_pos = next->pos;
			if (match_op(next->next, ':') &&
			    match_op(next->next->next, ':')) {
				struct token *attr = next->next->next->next;

				if (token_type(attr) == TOKEN_IDENT)
					sparse_error(
						id_pos,
						"unknown embed parameter '%s::%s'",
						show_ident(id),
						show_ident(attr->ident));
				else
					sparse_error(attr->pos,
						     "identifier expected");
				return 1;
			}

			kind = classify_embed_param(id);
			if (kind == EMBED_PARAM_NONE) {
				sparse_error(id_pos,
					     "unknown embed parameter '%s'",
					     show_ident(id));
				return 1;
			}
			if (seen_params & kind) {
				sparse_error(id_pos,
					     "duplicate embed parameter '%s'",
					     show_ident(id));
				return 1;
			}
			seen_params |= kind;

			next = scan_next(&next->next);
			if (!match_op(next, '(')) {
				sparse_error(id_pos, "expected '(' after '%s'",
					     show_ident(id));
				return 1;
			}
			pstart = next->next;
			next = next->next;
			while (!eof_token(next) && depth > 0) {
				if (match_op(next, '('))
					depth++;
				else if (match_op(next, ')') && !--depth)
					break;
				pend = next;
				next = next->next;
			}
			if (!match_op(next, ')')) {
				sparse_error(id_pos, "expected ')'");
				return 1;
			}
			next = next->next;
			if (kind == EMBED_PARAM_LIMIT) {
				if (!eval_embed_limit_expr(id_pos, pstart, pend,
							   &limit_val))
					return 1;
				has_limit = 1;
			} else if (kind == EMBED_PARAM_PREFIX) {
				prefix_start = pstart;
				prefix_end = pend;
			} else if (kind == EMBED_PARAM_SUFFIX) {
				suffix_start = pstart;
				suffix_end = pend;
			} else if (kind == EMBED_PARAM_IF_EMPTY) {
				if_empty_start = pstart;
				if_empty_end = pend;
			}
			param_p = &next;
			continue;
		}
		sparse_error(next->pos, "unexpected token in #embed");
		return 1;
	}

	fd = open_embed_file(stream, filename, expect);
	if (fd < 0) {
		sparse_error(token->pos, "unable to open '%s'", filename);
		return 1;
	}

	if (fstat(fd, &st) < 0) {
		close(fd);
		sparse_error(token->pos, "unable to stat '%s'", filename);
		return 1;
	}
	if (S_ISDIR(st.st_mode)) {
		close(fd);
		sparse_error(token->pos, "'%s' is a directory", filename);
		return 1;
	}

	if (S_ISREG(st.st_mode) || has_limit) {
		unsigned long long to_read =
			S_ISREG(st.st_mode) ? (unsigned long long)st.st_size :
					      limit_val;

		if (has_limit && limit_val < to_read)
			to_read = limit_val;
		if (to_read > MAX_EMBED_SIZE) {
			sparse_error(
				token->pos,
				"'%s' is too large to embed (%llu bytes, max %d bytes)",
				filename, to_read, MAX_EMBED_SIZE);
			close(fd);
			return 1;
		}
		buf = malloc(to_read ? to_read : 1);
		if (!buf) {
			sparse_error(
				token->pos,
				"out of memory allocating #embed buffer for '%s'",
				filename);
			close(fd);
			return 1;
		}
		if (to_read > 0) {
			size_t total = 0;

			while (total < to_read) {
				ssize_t r =
					read(fd, buf + total, to_read - total);
				if (r < 0) {
					if (errno == EINTR || errno == EAGAIN)
						continue;
					sparse_error(token->pos,
						     "error reading '%s': %s",
						     filename, strerror(errno));
					free(buf);
					close(fd);
					return 1;
				}
				if (r == 0)
					break;
				total += r;
			}
			bytes_read = total;
		}
	} else {
		size_t cap = 4096;
		size_t total = 0;

		buf = malloc(cap);
		if (!buf) {
			sparse_error(
				token->pos,
				"out of memory allocating #embed buffer for '%s'",
				filename);
			close(fd);
			return 1;
		}
		for (;;) {
			ssize_t r;

			if (total == cap) {
				size_t new_cap = cap * 2;
				unsigned char *nbuf;

				if (new_cap > MAX_EMBED_SIZE + 1)
					new_cap = MAX_EMBED_SIZE + 1;
				nbuf = realloc(buf, new_cap);
				if (!nbuf) {
					sparse_error(
						token->pos,
						"out of memory allocating #embed buffer for '%s'",
						filename);
					free(buf);
					close(fd);
					return 1;
				}
				buf = nbuf;
				cap = new_cap;
			}
			r = read(fd, buf + total, cap - total);
			if (r < 0) {
				if (errno == EINTR || errno == EAGAIN)
					continue;
				sparse_error(token->pos,
					     "error reading '%s': %s",
					     filename, strerror(errno));
				free(buf);
				close(fd);
				return 1;
			}
			if (r == 0)
				break;
			total += r;
			if (total > MAX_EMBED_SIZE) {
				sparse_error(
					token->pos,
					"'%s' is too large to embed (max %d bytes)",
					filename, MAX_EMBED_SIZE);
				free(buf);
				close(fd);
				return 1;
			}
		}
		bytes_read = total;
	}
	close(fd);

	if (bytes_read == 0) {
		if (if_empty_start && if_empty_end) {
			struct token *p = if_empty_start;
			while (p) {
				struct token *t = dup_token(p, &token->pos);
				*tail = t;
				tail = &t->next;
				if (p == if_empty_end)
					break;
				p = p->next;
			}
			*tail = *line;
			*line = head;
		}
		free(buf);
		free_preprocessor_line(token);
		return 0;
	}

	if (prefix_start && prefix_end) {
		struct token *p = prefix_start;
		while (p) {
			struct token *t = dup_token(p, &token->pos);
			*tail = t;
			tail = &t->next;
			if (p == prefix_end)
				break;
			p = p->next;
		}
	}

	if (!num_strs_inited) {
		for (unsigned int i = 0; i < 256; i++)
			snprintf(num_strs[i], sizeof(num_strs[i]), "%u",
				 i & 0xff);
		num_strs_inited = 1;
	}

	for (ssize_t i = 0; i < bytes_read; i++) {
		if (i > 0) {
			struct token *comma = alloc_token(&token->pos);
			token_type(comma) = TOKEN_SPECIAL;
			comma->special = ',';
			*tail = comma;
			tail = &comma->next;
		}
		struct token *num = alloc_token(&token->pos);
		token_type(num) = TOKEN_NUMBER;
		num->number = num_strs[buf[i]];
		*tail = num;
		tail = &num->next;
	}

	if (suffix_start && suffix_end) {
		struct token *p = suffix_start;
		while (p) {
			struct token *t = dup_token(p, &token->pos);
			*tail = t;
			tail = &t->next;
			if (p == suffix_end)
				break;
			p = p->next;
		}
	}

	if (head) {
		*tail = *line;
		*line = head;
	}
	free(buf);
	free_preprocessor_line(token);
	return 0;
}

static int free_preprocessor_line(struct token *token)
{
	while (token_type(token) != TOKEN_EOF) {
		struct token *free = token;
		token = token->next;
		__free_token(free);
	};
	return 1;
}

static int handle_include_path(struct stream *stream, struct token **list, struct token *token, int how)
{
	const char *filename;
	struct token *next;
	const char **path;
	int expect;
	int flen;

	next = token->next;
	expect = '>';
	if (!match_op(next, '<')) {
		expand_list(&token->next);
		expect = 0;
		next = token;
		if (match_op(token->next, '<')) {
			next = token->next;
			expect = '>';
		}
	}

	token = next->next;
	filename = token_name_sequence(token, expect, token);
	if (!filename)
		return 1;
	flen = strlen(filename) + 1;

	/* Absolute path? */
	if (filename[0] == '/') {
		if (try_include(token->pos, "", filename, flen, list, includepath))
			return 0;
		goto out;
	}

	switch (how) {
	case 1:
		path = stream->next_path;
		break;
	case 2:
		includepath[0] = "";
		path = includepath;
		break;
	default:
		/* Dir of input file is first dir to search for quoted includes */
		set_stream_include_path(stream);
		path = expect ? angle_includepath : quote_includepath;
		break;
	}
	/* Check the standard include paths.. */
	if (do_include_path(path, list, token, filename, flen))
		return 0;
out:
	error_die(token->pos, "unable to open '%s'", filename);
}

static int handle_include(struct stream *stream, struct token **list, struct token *token)
{
	return handle_include_path(stream, list, token, 0);
}

static int handle_include_next(struct stream *stream, struct token **list, struct token *token)
{
	return handle_include_path(stream, list, token, 1);
}

static int handle_argv_include(struct stream *stream, struct token **list, struct token *token)
{
	return handle_include_path(stream, list, token, 2);
}

static int token_list_different(struct token *list1, struct token *list2);

static int token_different(struct token *t1, struct token *t2)
{
	int different;

	if (token_type(t1) != token_type(t2))
		return 1;

	switch (token_type(t1)) {
	case TOKEN_IDENT:
		different = t1->ident != t2->ident;
		break;
	case TOKEN_ARG_COUNT:
	case TOKEN_UNTAINT:
	case TOKEN_CONCAT:
	case TOKEN_GNU_KLUDGE:
		different = 0;
		break;
	case TOKEN_VA_OPT:
	case TOKEN_STR_VA_OPT:
		different = token_list_different(t1->target, t2->target);
		break;
	case TOKEN_NUMBER:
		different = strcmp(t1->number, t2->number);
		break;
	case TOKEN_SPECIAL:
		different = t1->special != t2->special;
		break;
	case TOKEN_MACRO_ARGUMENT:
	case TOKEN_QUOTED_ARGUMENT:
	case TOKEN_STR_ARGUMENT:
		different = t1->argnum != t2->argnum;
		break;
	case TOKEN_CHAR_EMBEDDED_0 ... TOKEN_CHAR_EMBEDDED_3:
	case TOKEN_WIDE_CHAR_EMBEDDED_0 ... TOKEN_WIDE_CHAR_EMBEDDED_3:
	case TOKEN_UTF8_CHAR_EMBEDDED_0 ... TOKEN_UTF8_CHAR_EMBEDDED_3:
		different = memcmp(t1->embedded, t2->embedded, 4);
		break;
	case TOKEN_CHAR:
	case TOKEN_WIDE_CHAR:
	case TOKEN_UTF8_CHAR:
	case TOKEN_STRING:
	case TOKEN_WIDE_STRING:
	case TOKEN_UTF8_STRING: {
		struct string *s1, *s2;

		s1 = t1->string;
		s2 = t2->string;
		different = 1;
		if (s1->length != s2->length)
			break;
		different = memcmp(s1->data, s2->data, s1->length);
		break;
	}
	default:
		different = 1;
		break;
	}
	return different;
}

static int token_list_different(struct token *list1, struct token *list2)
{
	for (;;) {
		if (list1 == list2)
			return 0;
		if (!list1 || !list2)
			return 1;
		if (token_different(list1, list2))
			return 1;
		list1 = list1->next;
		list2 = list2->next;
	}
}

static inline void set_arg_count(struct token *token)
{
	token_type(token) = TOKEN_ARG_COUNT;
	token->count.normal = token->count.quoted =
	token->count.str = token->count.vararg = 0;
}

static struct token *parse_arguments(struct token *list)
{
	struct token *arg = list->next, *next = list;
	struct argcount *count = &list->count;

	set_arg_count(list);

	if (match_op(arg, ')')) {
		next = arg->next;
		list->next = &eof_token_entry;
		return next;
	}

	while (token_type(arg) == TOKEN_IDENT) {
		if (arg->ident == &__VA_ARGS___ident)
			goto Eva_args;
		if (arg->ident == &__VA_OPT___ident)
			goto Eva_opt;
		if (!++count->normal)
			goto Eargs;
		next = arg->next;

		if (match_op(next, ',')) {
			set_arg_count(next);
			arg = next->next;
			continue;
		}

		if (match_op(next, ')')) {
			set_arg_count(next);
			next = next->next;
			arg->next->next = &eof_token_entry;
			return next;
		}

		/* normal cases are finished here */

		if (match_op(next, SPECIAL_ELLIPSIS)) {
			if (match_op(next->next, ')')) {
				set_arg_count(next);
				next->count.vararg = 1;
				count->vararg = 1;
				next = next->next;
				arg->next->next = &eof_token_entry;
				return next->next;
			}

			arg = next;
			goto Enotclosed;
		}

		if (eof_token(next)) {
			goto Enotclosed;
		} else {
			arg = next;
			goto Ebadstuff;
		}
	}

	if (match_op(arg, SPECIAL_ELLIPSIS)) {
		next = arg->next;
		token_type(arg) = TOKEN_IDENT;
		arg->ident = &__VA_ARGS___ident;
		if (!match_op(next, ')'))
			goto Enotclosed;
		if (!++count->normal)
			goto Eargs;
		set_arg_count(next);
		next->count.vararg = 1;
		count->vararg = 1;
		next = next->next;
		arg->next->next = &eof_token_entry;
		return next;
	}

	if (eof_token(arg)) {
		arg = next;
		goto Enotclosed;
	}
	if (match_op(arg, ','))
		goto Emissing;
	else
		goto Ebadstuff;


Emissing:
	sparse_error(arg->pos, "parameter name missing");
	return NULL;
Ebadstuff:
	sparse_error(arg->pos, "\"%s\" may not appear in macro parameter list",
		show_token(arg));
	return NULL;
Enotclosed:
	sparse_error(arg->pos, "missing ')' in macro parameter list");
	return NULL;
Eva_opt:
	sparse_error(arg->pos,
		     "__VA_OPT__ cannot appear in a macro parameter list");
	return NULL;
Eva_args:
	sparse_error(arg->pos, "__VA_ARGS__ can only appear in the expansion of a C99 variadic macro");
	return NULL;
Eargs:
	sparse_error(arg->pos, "too many arguments in macro definition");
	return NULL;
}

static int try_arg(struct token *token, enum token_type type, struct token *arglist)
{
	struct ident *ident = token->ident;
	int nr;

	if (!arglist || token_type(token) != TOKEN_IDENT)
		return 0;

	arglist = arglist->next;

	for (nr = 0; !eof_token(arglist); nr++, arglist = arglist->next->next) {
		if (arglist->ident == ident) {
			struct argcount *count = &arglist->next->count;
			int n;

			token->argnum = nr;
			token_type(token) = type;
			switch (type) {
			case TOKEN_MACRO_ARGUMENT:
				n = ++count->normal;
				break;
			case TOKEN_QUOTED_ARGUMENT:
				n = ++count->quoted;
				break;
			default:
				n = ++count->str;
			}
			if (n)
				return count->vararg ? 2 : 1;
			/*
			 * XXX - need saner handling of that
			 * (>= 1024 instances of argument)
			 */
			token_type(token) = TOKEN_ERROR;
			return -1;
		}
	}
	return 0;
}

static struct token *handle_va_opt(struct token **p, struct token *arglist);
static int parse_expansion_tokens(struct token **expansion_p,
				  struct token *arglist);

static struct token *handle_hash(struct token **p, struct token *arglist)
{
	struct token *token = *p;
	if (arglist) {
		struct token *next = token->next;
		if (token_type(next) == TOKEN_IDENT &&
		    next->ident == &__VA_OPT___ident) {
			next = handle_va_opt(&token->next, arglist);
			if (!next)
				return NULL;
			token_type(next) = TOKEN_STR_VA_OPT;
			next->pos.whitespace = token->pos.whitespace;
			__free_token(token);
			*p = next;
			return next;
		}
		if (!try_arg(next, TOKEN_STR_ARGUMENT, arglist))
			goto Equote;
		next->pos.whitespace = token->pos.whitespace;
		__free_token(token);
		token = *p = next;
	} else {
		token->pos.noexpand = 1;
	}
	return token;

Equote:
	sparse_error(token->pos, "'#' is not followed by a macro parameter");
	return NULL;
}

/* token->next is ## */
static struct token *handle_hashhash(struct token *token, struct token *arglist)
{
	struct token *last = token;
	struct token *concat;
	int state = match_op(token, ',');
	
	try_arg(token, TOKEN_QUOTED_ARGUMENT, arglist);

	while (1) {
		struct token *t;
		int is_arg;

		/* eat duplicate ## */
		concat = token->next;
		while (match_op(t = concat->next, SPECIAL_HASHHASH)) {
			token->next = t;
			__free_token(concat);
			concat = t;
		}
		token_type(concat) = TOKEN_CONCAT;

		if (eof_token(t))
			goto Econcat;

		if (match_op(t, '#')) {
			t = handle_hash(&concat->next, arglist);
			if (!t)
				return NULL;
		} else if (token_type(t) == TOKEN_IDENT &&
			   t->ident == &__VA_OPT___ident) {
			t = handle_va_opt(&concat->next, arglist);
			if (!t)
				return NULL;
		}

		is_arg = try_arg(t, TOKEN_QUOTED_ARGUMENT, arglist);

		if (state == 1 && is_arg) {
			state = is_arg;
		} else {
			last = t;
			state = match_op(t, ',');
		}

		token = t;
		if (!match_op(token->next, SPECIAL_HASHHASH))
			break;
	}
	/* handle GNU ,##__VA_ARGS__ kludge, in all its weirdness */
	if (state == 2)
		token_type(last) = TOKEN_GNU_KLUDGE;
	return token;

Econcat:
	sparse_error(concat->pos, "'##' cannot appear at the ends of macro expansion");
	return NULL;
}

static struct token *handle_va_opt(struct token **p, struct token *arglist)
{
	struct token *token = *p;
	struct token *lparen = token->next;
	struct token *cur, *prev, *body;
	int nesting = 1;

	if (!arglist || !arglist->count.vararg) {
		sparse_error(
			token->pos,
			"__VA_OPT__ can only appear in the expansion of a variadic macro");
		return NULL;
	}
	if (!match_op(lparen, '(')) {
		sparse_error(token->pos, "expected '(' after __VA_OPT__");
		return NULL;
	}
	if (standard < STANDARD_C23 && Wpedantic)
		warning(token->pos, "__VA_OPT__ is a C23 feature");

	for (cur = arglist->next; !eof_token(cur); cur = cur->next->next) {
		if (cur->next->count.vararg) {
			if (!++cur->next->count.normal) {
				sparse_error(
					token->pos,
					"too many instances of argument in body");
				return NULL;
			}
			break;
		}
	}

	prev = lparen;
	for (cur = lparen->next; !eof_token(cur); prev = cur, cur = cur->next) {
		if (token_type(cur) == TOKEN_IDENT &&
		    cur->ident == &__VA_OPT___ident) {
			sparse_error(
				cur->pos,
				"__VA_OPT__ cannot appear within __VA_OPT__");
			return NULL;
		}
		if (match_op(cur, '('))
			nesting++;
		else if (match_op(cur, ')')) {
			if (!--nesting)
				break;
		}
	}
	if (nesting) {
		sparse_error(token->pos, "unterminated __VA_OPT__");
		return NULL;
	}

	/* cur is ')' */
	if (lparen->next != cur && match_op(lparen->next, SPECIAL_HASHHASH)) {
		sparse_error(
			lparen->next->pos,
			"'##' cannot appear at the beginning of __VA_OPT__");
		return NULL;
	}
	if (prev != lparen && match_op(prev, SPECIAL_HASHHASH)) {
		sparse_error(prev->pos,
			     "'##' cannot appear at the end of __VA_OPT__");
		return NULL;
	}

	if (lparen->next == cur) {
		body = &eof_token_entry;
	} else {
		body = lparen->next;
		prev->next = &eof_token_entry;
		if (parse_expansion_tokens(&body, arglist) < 0)
			return NULL;
	}

	token_type(token) = TOKEN_VA_OPT;
	token->target = body;
	token->next = cur->next;
	__free_token(lparen);
	__free_token(cur);
	return token;
}

static int parse_expansion_tokens(struct token **expansion_p,
				  struct token *arglist)
{
	struct token *token = *expansion_p;
	struct token **p;

	if (match_op(token, SPECIAL_HASHHASH))
		goto Econcat;

	for (p = expansion_p; !eof_token(token); p = &token->next, token = *p) {
		if (match_op(token, '#')) {
			token = handle_hash(p, arglist);
			if (!token)
				return -1;
		} else if (token_type(token) == TOKEN_IDENT &&
			   token->ident == &__VA_OPT___ident) {
			token = handle_va_opt(p, arglist);
			if (!token)
				return -1;
		}
		if (match_op(token->next, SPECIAL_HASHHASH)) {
			token = handle_hashhash(token, arglist);
			if (!token)
				return -1;
		} else if (token_type(token) == TOKEN_IDENT) {
			try_arg(token, TOKEN_MACRO_ARGUMENT, arglist);
		}
		if (token_type(token) == TOKEN_ERROR)
			goto Earg;
	}
	return 0;

Econcat:
	sparse_error(token->pos,
		     "'##' cannot appear at the ends of macro expansion");
	return -1;
Earg:
	sparse_error(token->pos, "too many instances of argument in body");
	return -1;
}

static struct token *parse_expansion(struct token *expansion, struct token *arglist, struct ident *name)
{
	struct token *token;
	struct token **p;

	if (parse_expansion_tokens(&expansion, arglist) < 0)
		return NULL;

	for (p = &expansion; !eof_token(*p); p = &(*p)->next)
		;
	token = alloc_token(&expansion->pos);
	token_type(token) = TOKEN_UNTAINT;
	token->ident = name;
	token->next = *p;
	*p = token;
	return expansion;
}

static int do_define(struct position pos, struct token *token, struct ident *name,
		     struct token *arglist, struct token *expansion, int attr)
{
	struct symbol *sym;
	int ret = 1;

	if (name == &__VA_ARGS___ident || name == &__VA_OPT___ident) {
		sparse_error(pos, "\"%s\" cannot be used as a macro name",
			     show_ident(name));
		return 1;
	}

	expansion = parse_expansion(expansion, arglist, name);
	if (!expansion)
		return 1;

	sym = lookup_symbol(name, NS_MACRO | NS_UNDEF);
	if (sym) {
		int clean;

		if (attr < sym->attr)
			goto out;

		clean = (attr == sym->attr && sym->namespace == NS_MACRO);

		if (token_list_different(sym->expansion, expansion) ||
		    token_list_different(sym->arglist, arglist)) {
			ret = 0;
			if ((clean && attr == SYM_ATTR_NORMAL)
					|| sym->used_in == file_scope) {
				warning(pos, "preprocessor token %.*s redefined",
						name->len, name->name);
				info(sym->pos, "this was the original definition");
			}
		} else if (clean)
			goto out;
	}

	if (!sym || sym->scope != file_scope) {
		sym = alloc_symbol(pos, SYM_NODE);
		bind_symbol(sym, name, NS_MACRO);
		add_ident(&macros, name);
		ret = 0;
	}

	if (!ret) {
		sym->expansion = expansion;
		sym->arglist = arglist;
		if (token) /* Free the "define" token, but not the rest of the line */
			__free_token(token);
	}

	sym->namespace = NS_MACRO;
	sym->used_in = NULL;
	sym->attr = attr;
out:
	return ret;
}

///
// predefine a macro with a printf-formatted value
// @name: the name of the macro
// @weak: 0/1 for a normal or a weak define
// @fmt: the printf format followed by it's arguments.
//
// The type of the value is automatically infered:
// TOKEN_NUMBER if it starts by a digit, TOKEN_IDENT otherwise.
// If @fmt is null or empty, the macro is defined with an empty definition.
void predefine(const char *name, int weak, const char *fmt, ...)
{
	struct ident *ident = built_in_ident(name);
	struct token *value = &eof_token_entry;
	int attr = weak ? SYM_ATTR_WEAK : SYM_ATTR_NORMAL;

	if (fmt && fmt[0]) {
		static char buf[256];
		va_list ap;

		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		value = __alloc_token(0);
		if (isdigit((unsigned char)buf[0])) {
			token_type(value) = TOKEN_NUMBER;
			value->number = xstrdup(buf);
		} else {
			token_type(value) = TOKEN_IDENT;
			value->ident = built_in_ident(buf);
		}
		value->pos.whitespace = 1;
		value->next = &eof_token_entry;
	}

	do_define(value->pos, NULL, ident, NULL, value, attr);
}

///
// like predefine() but only if one of the non-standard dialect is chosen
void predefine_nostd(const char *name)
{
	if ((standard & STANDARD_GNU) || (standard == STANDARD_NONE))
		predefine(name, 1, "1");
}

static void predefine_fmt(const char *fmt, int weak, va_list ap)
{
	static char buf[256];

	vsnprintf(buf, sizeof(buf), fmt, ap);
	predefine(buf, weak, "1");
}

void predefine_strong(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	predefine_fmt(fmt, 0, ap);
	va_end(ap);
}

void predefine_weak(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	predefine_fmt(fmt, 1, ap);
	va_end(ap);
}

static int do_handle_define(struct stream *stream, struct token **line, struct token *token, int attr)
{
	struct token *arglist, *expansion;
	struct token *left = token->next;
	struct ident *name;

	if (token_type(left) != TOKEN_IDENT) {
		sparse_error(token->pos, "expected identifier to 'define'");
		return 1;
	}

	name = left->ident;

	arglist = NULL;
	expansion = left->next;
	if (!expansion->pos.whitespace) {
		if (match_op(expansion, '(')) {
			arglist = expansion;
			expansion = parse_arguments(expansion);
			if (!expansion)
				return 1;
		} else if (!eof_token(expansion)) {
			warning(expansion->pos,
				"no whitespace before object-like macro body");
		}
	}

	return do_define(left->pos, token, name, arglist, expansion, attr);
}

static int handle_define(struct stream *stream, struct token **line, struct token *token)
{
	return do_handle_define(stream, line, token, SYM_ATTR_NORMAL);
}

static int handle_weak_define(struct stream *stream, struct token **line, struct token *token)
{
	return do_handle_define(stream, line, token, SYM_ATTR_WEAK);
}

static int handle_strong_define(struct stream *stream, struct token **line, struct token *token)
{
	return do_handle_define(stream, line, token, SYM_ATTR_STRONG);
}

static int do_handle_undef(struct stream *stream, struct token **line, struct token *token, int attr)
{
	struct token *left = token->next;
	struct symbol *sym;

	if (token_type(left) != TOKEN_IDENT) {
		sparse_error(token->pos, "expected identifier to 'undef'");
		return 1;
	}

	if (left->ident == &__VA_ARGS___ident ||
	    left->ident == &__VA_OPT___ident) {
		warning(token->pos, "undefining \"%s\"",
			show_ident(left->ident));
		return 1;
	}

	sym = lookup_symbol(left->ident, NS_MACRO | NS_UNDEF);
	if (sym) {
		if (attr < sym->attr)
			return 1;
		if (attr == sym->attr && sym->namespace == NS_UNDEF)
			return 1;
	} else if (attr <= SYM_ATTR_NORMAL)
		return 1;

	if (!sym || sym->scope != file_scope) {
		sym = alloc_symbol(left->pos, SYM_NODE);
		bind_symbol(sym, left->ident, NS_MACRO);
	}

	sym->namespace = NS_UNDEF;
	sym->used_in = NULL;
	sym->attr = attr;

	return 1;
}

static int handle_undef(struct stream *stream, struct token **line, struct token *token)
{
	return do_handle_undef(stream, line, token, SYM_ATTR_NORMAL);
}

static int handle_strong_undef(struct stream *stream, struct token **line, struct token *token)
{
	return do_handle_undef(stream, line, token, SYM_ATTR_STRONG);
}

static int preprocessor_if(struct stream *stream, struct token *token, int cond)
{
	token_type(token) = false_nesting ? TOKEN_SKIP_GROUPS : TOKEN_IF;
	free_preprocessor_line(token->next);
	token->next = stream->top_if;
	stream->top_if = token;
	if (false_nesting || cond != 1)
		false_nesting++;
	return 0;
}

static int handle_ifdef(struct stream *stream, struct token **line, struct token *token)
{
	struct token *next = token->next;
	int arg;
	if (token_type(next) == TOKEN_IDENT) {
		arg = token_defined(next);
	} else {
		dirty_stream(stream);
		if (!false_nesting)
			sparse_error(token->pos, "expected preprocessor identifier");
		arg = -1;
	}
	return preprocessor_if(stream, token, arg);
}

static int handle_ifndef(struct stream *stream, struct token **line, struct token *token)
{
	struct token *next = token->next;
	int arg;
	if (token_type(next) == TOKEN_IDENT) {
		if (!stream->dirty && !stream->ifndef) {
			if (!stream->protect) {
				stream->ifndef = token;
				stream->protect = next->ident;
			} else if (stream->protect == next->ident) {
				stream->ifndef = token;
				stream->dirty = 1;
			}
		}
		arg = !token_defined(next);
	} else {
		dirty_stream(stream);
		if (!false_nesting)
			sparse_error(token->pos, "expected preprocessor identifier");
		arg = -1;
	}

	return preprocessor_if(stream, token, arg);
}

/*
 * Expression handling for #if and #elif; it differs from normal expansion
 * due to special treatment of "defined".
 */
static int expression_value(struct token **where)
{
	struct expression *expr;
	struct token *p;
	struct token **list = where, **beginning = NULL;
	long long value;
	int state = 0;

	in_cond_expr++;
	while (!eof_token(p = scan_next(list))) {
		switch (state) {
		case 0:
			if (token_type(p) != TOKEN_IDENT)
				break;
			if (p->ident == &defined_ident) {
				state = 1;
				beginning = list;
				break;
			}
			if (p->ident == &__has_include_ident) {
				if (parse_has_include(list, 0))
					continue;
				replace_with_bool(p, 0);
				break;
			}
			if (p->ident == &__has_include_next_ident) {
				if (parse_has_include(list, 1))
					continue;
				replace_with_bool(p, 0);
				break;
			}
			if (p->ident == &__has_embed_ident) {
				if (parse_has_embed(list))
					continue;
				replace_with_bool(p, 0);
				break;
			}
			if (!expand_one_symbol(list))
				continue;
			if (token_type(p) != TOKEN_IDENT)
				break;
			if (standard >= STANDARD_C23 &&
			    p->ident == &true_ident) {
				replace_with_bool(p, 1);
				break;
			}
			if (standard >= STANDARD_C23 &&
			    p->ident == &false_ident) {
				replace_with_bool(p, 0);
				break;
			}
			token_type(p) = TOKEN_ZERO_IDENT;
			break;
		case 1:
			if (match_op(p, '(')) {
				state = 2;
			} else {
				state = 0;
				replace_with_defined(p);
				*beginning = p;
			}
			break;
		case 2:
			if (token_type(p) == TOKEN_IDENT)
				state = 3;
			else
				state = 0;
			replace_with_defined(p);
			*beginning = p;
			break;
		case 3:
			state = 0;
			if (!match_op(p, ')'))
				sparse_error(p->pos, "missing ')' after \"defined\"");
			*list = p->next;
			continue;
		}
		list = &p->next;
	}

	if (state == 1 || state == 2) {
		sparse_error((*beginning)->pos, "expected identifier after \"defined\"");
		replace_with_bool(*beginning, 0);
		(*beginning)->next = &eof_token_entry;
	} else if (state == 3) {
		sparse_error((*beginning)->pos, "missing ')' after \"defined\"");
	}

	p = constant_expression(*where, &expr);
	if (!eof_token(p))
		sparse_error(p->pos, "garbage at end: %s", show_token_sequence(p, 0));
	value = get_expression_value(expr);
	in_cond_expr--;
	return value != 0;
}

static int handle_if(struct stream *stream, struct token **line, struct token *token)
{
	int value = 0;
	if (!false_nesting)
		value = expression_value(&token->next);

	dirty_stream(stream);
	return preprocessor_if(stream, token, value);
}

static int do_handle_elif(struct stream *stream, struct token *token,
			  int (*eval)(struct token *),
			  bool (*check)(struct token *))
{
	struct token *top_if = stream->top_if;
	end_group(stream);

	if (!top_if) {
		nesting_error(stream);
		sparse_error(token->pos, "unmatched #%s within stream",
			     show_token(token));
		return 1;
	}

	if (token_type(top_if) == TOKEN_ELSE) {
		nesting_error(stream);
		sparse_error(token->pos, "#%s after #else", show_token(token));
		if (!false_nesting)
			false_nesting = 1;
		return 1;
	}

	dirty_stream(stream);
	if (token_type(top_if) != TOKEN_IF) {
		if (false_nesting <= 1 && check)
			check(token);
		return 1;
	}
	if (false_nesting) {
		false_nesting = 0;
		if (!eval(token))
			false_nesting = 1;
	} else {
		if (check)
			check(token);
		false_nesting = 1;
		token_type(top_if) = TOKEN_SKIP_GROUPS;
	}
	return 1;
}

static int eval_elif(struct token *token)
{
	return expression_value(&token->next);
}

static bool check_elifdef(struct token *token)
{
	struct token *next = token->next;

	if (standard < STANDARD_C23 && Wpedantic)
		warning(token->pos, "ISO C does not support #%s before C23",
			show_token(token));
	if (token_type(next) != TOKEN_IDENT) {
		sparse_error(token->pos, "expected preprocessor identifier");
		return false;
	}
	if (!eof_token(next->next))
		warning(next->next->pos,
			"extra tokens at end of #%s directive",
			show_token(token));
	return true;
}

static int eval_elifdef(struct token *token)
{
	return check_elifdef(token) && token_defined(token->next);
}

static int eval_elifndef(struct token *token)
{
	return check_elifdef(token) && !token_defined(token->next);
}

static int handle_elif(struct stream *stream, struct token **line,
		       struct token *token)
{
	return do_handle_elif(stream, token, eval_elif, NULL);
}

static int handle_elifdef(struct stream *stream, struct token **line,
			  struct token *token)
{
	return do_handle_elif(stream, token, eval_elifdef, check_elifdef);
}

static int handle_elifndef(struct stream *stream, struct token **line,
			   struct token *token)
{
	return do_handle_elif(stream, token, eval_elifndef, check_elifdef);
}

static int handle_else(struct stream *stream, struct token **line, struct token *token)
{
	struct token *top_if = stream->top_if;
	end_group(stream);

	if (!top_if) {
		nesting_error(stream);
		sparse_error(token->pos, "unmatched #else within stream");
		return 1;
	}

	if (token_type(top_if) == TOKEN_ELSE) {
		nesting_error(stream);
		sparse_error(token->pos, "#else after #else");
	}
	if (false_nesting) {
		if (token_type(top_if) == TOKEN_IF)
			false_nesting = 0;
	} else {
		false_nesting = 1;
	}
	token_type(top_if) = TOKEN_ELSE;
	return 1;
}

static int handle_endif(struct stream *stream, struct token **line, struct token *token)
{
	struct token *top_if = stream->top_if;
	end_group(stream);
	if (!top_if) {
		nesting_error(stream);
		sparse_error(token->pos, "unmatched #endif in stream");
		return 1;
	}
	if (false_nesting)
		false_nesting--;
	stream->top_if = top_if->next;
	__free_token(top_if);
	return 1;
}

static int handle_warning(struct stream *stream, struct token **line, struct token *token)
{
	warning(token->pos, "%s", show_token_sequence(token->next, 0));
	return 1;
}

static int handle_error(struct stream *stream, struct token **line, struct token *token)
{
	sparse_error(token->pos, "%s", show_token_sequence(token->next, 0));
	return 1;
}

static int handle_nostdinc(struct stream *stream, struct token **line, struct token *token)
{
	/*
	 * Do we have any non-system includes?
	 * Clear them out if so..
	 */
	*sys_includepath = NULL;
	return 1;
}

static inline void update_inc_ptrs(const char ***where)
{

	if (*where <= dirafter_includepath) {
		dirafter_includepath++;
		/* If this was the entry that we prepend, don't
		 * rise the lower entries, even if they are at
		 * the same level. */
		if (where == &dirafter_includepath)
			return;
	}
	if (*where <= sys_includepath) {
		sys_includepath++;
		if (where == &sys_includepath)
			return;
	}
	if (*where <= isys_includepath) {
		isys_includepath++;
		if (where == &isys_includepath)
			return;
	}

	/* angle_includepath is actually never updated, since we
	 * don't suppport -iquote rught now. May change some day. */
	if (*where <= angle_includepath) {
		angle_includepath++;
		if (where == &angle_includepath)
			return;
	}
}

/* Add a path before 'where' and update the pointers associated with the
 * includepath array */
static void add_path_entry(struct token *token, const char *path,
	const char ***where)
{
	const char **dst;
	const char *next;

	/* Need one free entry.. */
	if (includepath[INCLUDEPATHS-2])
		error_die(token->pos, "too many include path entries");

	/* check that this is not a duplicate */
	dst = includepath;
	while (*dst) {
		if (strcmp(*dst, path) == 0)
			return;
		dst++;
	}
	next = path;
	dst = *where;

	update_inc_ptrs(where);

	/*
	 * Move them all up starting at dst,
	 * insert the new entry..
	 */
	do {
		const char *tmp = *dst;
		*dst = next;
		next = tmp;
		dst++;
	} while (next);
}

static int handle_add_include(struct stream *stream, struct token **line, struct token *token)
{
	for (;;) {
		token = token->next;
		if (eof_token(token))
			return 1;
		if (token_type(token) != TOKEN_STRING) {
			warning(token->pos, "expected path string");
			return 1;
		}
		add_path_entry(token, token->string->data, &isys_includepath);
	}
}

static int handle_add_isystem(struct stream *stream, struct token **line, struct token *token)
{
	for (;;) {
		token = token->next;
		if (eof_token(token))
			return 1;
		if (token_type(token) != TOKEN_STRING) {
			sparse_error(token->pos, "expected path string");
			return 1;
		}
		add_path_entry(token, token->string->data, &sys_includepath);
	}
}

static int handle_add_system(struct stream *stream, struct token **line, struct token *token)
{
	for (;;) {
		token = token->next;
		if (eof_token(token))
			return 1;
		if (token_type(token) != TOKEN_STRING) {
			sparse_error(token->pos, "expected path string");
			return 1;
		}
		add_path_entry(token, token->string->data, &dirafter_includepath);
	}
}

/* Add to end on includepath list - no pointer updates */
static void add_dirafter_entry(struct token *token, const char *path)
{
	const char **dst = includepath;

	/* Need one free entry.. */
	if (includepath[INCLUDEPATHS-2])
		error_die(token->pos, "too many include path entries");

	/* Add to the end */
	while (*dst)
		dst++;
	*dst = path;
	dst++;
	*dst = NULL;
}

static int handle_add_dirafter(struct stream *stream, struct token **line, struct token *token)
{
	for (;;) {
		token = token->next;
		if (eof_token(token))
			return 1;
		if (token_type(token) != TOKEN_STRING) {
			sparse_error(token->pos, "expected path string");
			return 1;
		}
		add_dirafter_entry(token, token->string->data);
	}
}

static int handle_split_include(struct stream *stream, struct token **line, struct token *token)
{
	/*
	 * -I-
	 *  From info gcc:
	 *  Split the include path.  Any directories specified with `-I'
	 *  options before `-I-' are searched only for headers requested with
	 *  `#include "FILE"'; they are not searched for `#include <FILE>'.
	 *  If additional directories are specified with `-I' options after
	 *  the `-I-', those directories are searched for all `#include'
	 *  directives.
	 *  In addition, `-I-' inhibits the use of the directory of the current
	 *  file directory as the first search directory for `#include "FILE"'.
	 */
	quote_includepath = includepath+1;
	angle_includepath = sys_includepath;
	return 1;
}

/*
 * We replace "#pragma xxx" with "__pragma__" in the token
 * stream. Just as an example.
 *
 * We'll just #define that away for now, but the theory here
 * is that we can use this to insert arbitrary token sequences
 * to turn the pragmas into internal front-end sequences for
 * when we actually start caring about them.
 *
 * So eventually this will turn into some kind of extended
 * __attribute__() like thing, except called __pragma__(xxx).
 */
static int handle_pragma(struct stream *stream, struct token **line, struct token *token)
{
	struct token *next = *line;

	if (match_ident(token->next, &once_ident) && eof_token(token->next->next)) {
		stream->once = 1;
		return 1;
	}
	token->ident = &pragma_ident;
	token->pos.newline = 1;
	token->pos.whitespace = 1;
	token->pos.pos = 1;
	*line = token;
	token->next = next;
	return 0;
}

/*
 * We ignore #line for now.
 */
static int handle_line(struct stream *stream, struct token **line, struct token *token)
{
	return 1;
}

static int handle_ident(struct stream *stream, struct token **line, struct token *token)
{
	return 1;
}

static int handle_nondirective(struct stream *stream, struct token **line, struct token *token)
{
	sparse_error(token->pos, "unrecognized preprocessor line '%s'", show_token_sequence(token, 0));
	return 1;
}

static bool expand_has_c_attribute(struct token *token, struct arg *args)
{
	struct token *arg = args[0].expanded;
	const char *name;
	const char *val = "0";
	char buf[4096];

	if (token_type(arg) != TOKEN_IDENT) {
		sparse_error(eof_token(arg) ? token->pos : arg->pos,
			     "identifier expected");
		return false;
	}

	if (match_op(arg->next, ':') && match_op(arg->next->next, ':')) {
		struct token *attr = arg->next->next->next;
		if (token_type(attr) != TOKEN_IDENT || !eof_token(attr->next)) {
			sparse_error(eof_token(attr) ? token->pos : attr->pos,
				     "identifier expected");
			return false;
		}
		if (arg->ident == &gnu_ident || arg->ident == &__gnu___ident) {
			struct symbol *sym = lookup_symbol(attr->ident, NS_KEYWORD);
			if (sym && sym->op && sym->op->attribute)
				val = "1";
		}
		token_type(token) = TOKEN_NUMBER;
		token->number = val;
		return true;
	}

	if (!eof_token(arg->next)) {
		sparse_error(arg->next->pos, "expected ')'");
		return false;
	}

	name = show_ident(arg->ident);
	if (name[0] == '_' && name[1] == '_') {
		size_t len = strlen(name);
		if (len > 4 && name[len - 1] == '_' && name[len - 2] == '_') {
			if (len - 4 < sizeof(buf)) {
				memcpy(buf, name + 2, len - 4);
				buf[len - 4] = '\0';
				name = buf;
			}
		}
	}

	if (!strcmp(name, "deprecated"))
		val = "201904L";
	else if (!strcmp(name, "fallthrough"))
		val = "201910L";
	else if (!strcmp(name, "nodiscard"))
		val = "202003L";
	else if (!strcmp(name, "maybe_unused"))
		val = "202106L";
	else if (!strcmp(name, "noreturn") || !strcmp(name, "_Noreturn"))
		val = "202202L";
	else if (!strcmp(name, "reproducible") || !strcmp(name, "unsequenced"))
		val = "202207L";

	token_type(token) = TOKEN_NUMBER;
	token->number = val;
	return true;
}

static bool expand_has_attribute(struct token *token, struct arg *args)
{
	struct token *arg = args[0].expanded;
	struct symbol *sym;

	if (token_type(arg) != TOKEN_IDENT) {
		sparse_error(arg->pos, "identifier expected");
		return false;
	}

	sym = lookup_symbol(arg->ident, NS_KEYWORD);
	replace_with_bool(token, sym && sym->op && sym->op->attribute);
	return true;
}

static bool expand_has_builtin(struct token *token, struct arg *args)
{
	struct token *arg = args[0].expanded;
	struct symbol *sym;

	if (token_type(arg) != TOKEN_IDENT) {
		sparse_error(arg->pos, "identifier expected");
		return false;
	}

	sym = lookup_symbol(arg->ident, NS_SYMBOL);
	replace_with_bool(token, sym && sym->builtin);
	return true;
}

static bool expand_has_extension(struct token *token, struct arg *args)
{
	struct token *arg = args[0].expanded;
	struct ident *ident;
	bool val = false;

	if (token_type(arg) != TOKEN_IDENT) {
		sparse_error(arg->pos, "identifier expected");
		return false;
	}

	ident = arg->ident;
	if (ident == &c_alignas_ident)
		val = true;
	else if (ident == &c_alignof_ident)
		val = true;
	else if (ident == &c_generic_selections_ident)
		val = true;
	else if (ident == &c_static_assert_ident)
		val = true;

	replace_with_bool(token, val);
	return 1;
}

static bool expand_has_feature(struct token *token, struct arg *args)
{
	struct token *arg = args[0].expanded;
	struct ident *ident;
	bool val = false;

	if (token_type(arg) != TOKEN_IDENT) {
		sparse_error(arg->pos, "identifier expected");
		return false;
	}

	ident = arg->ident;
	if (standard >= STANDARD_C11) {
		if (ident == &c_alignas_ident)
			val = true;
		else if (ident == &c_alignof_ident)
			val = true;
		else if (ident == &c_generic_selections_ident)
			val = true;
		else if (ident == &c_static_assert_ident)
			val = true;
	}

	replace_with_bool(token, val);
	return 1;
}

static void create_arglist(struct symbol *sym, int count)
{
	struct token *token;
	struct token **next;

	if (!count)
		return;

	token = __alloc_token(0);
	token_type(token) = TOKEN_ARG_COUNT;
	token->count.normal = count;
	sym->arglist = token;
	next = &token->next;

	while (count--) {
		struct token *id, *uses;
		id = __alloc_token(0);
		token_type(id) = TOKEN_IDENT;
		uses = __alloc_token(0);
		token_type(uses) = TOKEN_ARG_COUNT;
		uses->count.normal = 1;

		*next = id;
		id->next = uses;
		next = &uses->next;
	}
	*next = &eof_token_entry;
}

static void init_preprocessor(void)
{
	int i;
	int stream = init_stream(NULL, "preprocessor", -1, includepath);
	static struct {
		const char *name;
		int (*handler)(struct stream *, struct token **, struct token *);
	} normal[] = {
		{ "define",		handle_define },
		{ "weak_define",	handle_weak_define },
		{ "strong_define",	handle_strong_define },
		{ "undef",		handle_undef },
		{ "strong_undef",	handle_strong_undef },
		{ "warning",		handle_warning },
		{ "error",		handle_error },
		{ "include",		handle_include },
		{ "include_next",	handle_include_next },
		{ "embed",		handle_embed },
		{ "pragma",		handle_pragma },
		{ "line",		handle_line },
		{ "ident",		handle_ident },

		// our internal preprocessor tokens
		{ "nostdinc",	   handle_nostdinc },
		{ "add_include",   handle_add_include },
		{ "add_isystem",   handle_add_isystem },
		{ "add_system",    handle_add_system },
		{ "add_dirafter",  handle_add_dirafter },
		{ "split_include", handle_split_include },
		{ "argv_include",  handle_argv_include },
	}, special[] = {
		{ "ifdef",	handle_ifdef },
		{ "ifndef",	handle_ifndef },
		{ "else",	handle_else },
		{ "endif",	handle_endif },
		{ "if",		handle_if },
		{ "elif",	handle_elif },
		{ "elifdef",	handle_elifdef },
		{ "elifndef",	handle_elifndef },
	};
	static struct {
		const char *name;
		void (*expand_simple)(struct token *);
		bool (*expand)(struct token *, struct arg *args);
	} dynamic[] = {
		{ "__LINE__", expand_line },
		{ "__FILE__", expand_file },
		{ "__BASE_FILE__", expand_basefile },
		{ "__DATE__", expand_date },
		{ "__TIME__", expand_time },
		{ "__COUNTER__", expand_counter },
		{ "__INCLUDE_LEVEL__", expand_include_level },
		{ "__has_attribute", NULL, expand_has_attribute },
		{ "__has_builtin", NULL, expand_has_builtin },
		{ "__has_c_attribute", NULL, expand_has_c_attribute },
		{ "__has_extension", NULL, expand_has_extension },
		{ "__has_feature", NULL, expand_has_feature },
		{ "__has_include", NULL, NULL },
		{ "__has_include_next", NULL, NULL },
		{ "__has_embed", NULL, NULL },
	};

	for (i = 0; i < ARRAY_SIZE(normal); i++) {
		struct symbol *sym;
		sym = create_symbol(stream, normal[i].name, SYM_PREPROCESSOR, NS_PREPROCESSOR);
		sym->handler = normal[i].handler;
		sym->normal = 1;
	}
	for (i = 0; i < ARRAY_SIZE(special); i++) {
		struct symbol *sym;
		sym = create_symbol(stream, special[i].name, SYM_PREPROCESSOR, NS_PREPROCESSOR);
		sym->handler = special[i].handler;
		sym->normal = 0;
	}
	for (i = 0; i < ARRAY_SIZE(dynamic); i++) {
		struct symbol *sym;
		sym = create_symbol(stream, dynamic[i].name, SYM_NODE, NS_MACRO);
		sym->expand_simple = dynamic[i].expand_simple;
		if ((sym->expand = dynamic[i].expand) != NULL)
			create_arglist(sym, 1);
	}

	counter_macro = 0;
}

static void handle_preprocessor_line(struct stream *stream, struct token **line, struct token *start)
{
	int (*handler)(struct stream *, struct token **, struct token *);
	struct token *token = start->next;
	int is_normal = 1;
	int is_cond = 0;	// is one of {is,ifdef,ifndef,elif,else,endif}

	if (eof_token(token))
		return;

	if (token_type(token) == TOKEN_IDENT) {
		struct symbol *sym = lookup_symbol(token->ident, NS_PREPROCESSOR);
		if (sym) {
			handler = sym->handler;
			is_normal = sym->normal;
			is_cond = !sym->normal;
		} else {
			handler = handle_nondirective;
		}
	} else if (token_type(token) == TOKEN_NUMBER) {
		handler = handle_line;
	} else {
		handler = handle_nondirective;
	}

	if (is_normal) {
		dirty_stream(stream);
		if (false_nesting)
			goto out;
	}

	if (expanding) {
		if (!is_cond || Wpedantic)
			warning(start->pos, "directive in macro's argument list");
	}
	if (!handler(stream, line, token))	/* all set */
		return;

out:
	free_preprocessor_line(token);
}

static void preprocessor_line(struct stream *stream, struct token **line)
{
	struct token *start = *line, *next;
	struct token **tp = &start->next;

	for (;;) {
		next = *tp;
		if (next->pos.newline)
			break;
		tp = &next->next;
	}
	*line = next;
	*tp = &eof_token_entry;
	handle_preprocessor_line(stream, line, start);
}

static void do_preprocess(struct token **list)
{
	struct token *next;

	while (!eof_token(next = scan_next(list))) {
		struct stream *stream = input_streams + next->pos.stream;

		if (next->pos.newline && match_op(next, '#')) {
			if (!next->pos.noexpand) {
				preprocessor_line(stream, list);
				__free_token(next);	/* Free the '#' token */
				continue;
			}
		}

		switch (token_type(next)) {
		case TOKEN_STREAMEND:
			if (stream->top_if) {
				nesting_error(stream);
				sparse_error(stream->top_if->pos, "unterminated preprocessor conditional");
				stream->top_if = NULL;
				false_nesting = 0;
			}
			if (!stream->dirty)
				stream->constant = CONSTANT_FILE_YES;
			*list = next->next;
			include_level--;
			continue;
		case TOKEN_STREAMBEGIN:
			*list = next->next;
			include_level++;
			continue;

		default:
			dirty_stream(stream);
			if (false_nesting) {
				*list = next->next;
				__free_token(next);
				continue;
			}

			if (token_type(next) != TOKEN_IDENT ||
			    expand_one_symbol(list))
				list = &next->next;
		}
	}
}

struct token * preprocess(struct token *token)
{
	preprocessing = 1;
	init_preprocessor();
	do_preprocess(&token);

	// Drop all expressions from preprocessing, they're not used any more.
	// This is not true when we have multiple files, though ;/
	// clear_expression_alloc();
	preprocessing = 0;

	return token;
}

static int is_VA_ARGS_token(struct token *token)
{
	return (token_type(token) == TOKEN_IDENT) &&
		(token->ident == &__VA_ARGS___ident);
}

static void dump_macro_tokens(struct token *token, struct token **args,
			      bool top_level)
{
	bool first = !top_level;

	while (top_level ? token_type(token) != TOKEN_UNTAINT :
			   !eof_token(token)) {
		struct token *next = token->next;

		if (!first && token->pos.whitespace)
			putchar(' ');
		first = false;
		switch (token_type(token)) {
		case TOKEN_CONCAT:
			printf("##");
			break;
		case TOKEN_STR_VA_OPT:
			printf("#");
			/* fall-through */
		case TOKEN_VA_OPT:
			printf("__VA_OPT__(");
			dump_macro_tokens(token->target, args, false);
			putchar(')');
			break;
		case TOKEN_STR_ARGUMENT:
			printf("#");
			/* fall-through */
		case TOKEN_QUOTED_ARGUMENT:
		case TOKEN_MACRO_ARGUMENT:
			token = args[token->argnum];
			/* fall-through */
		default:
			printf("%s", show_token(token));
		}
		token = next;
	}
}

static void dump_macro(struct symbol *sym)
{
	int nargs = sym->arglist ? sym->arglist->count.normal : 0;
	struct token *args[nargs ? nargs : 1];
	struct token *token;

	printf("#define %s", show_ident(sym->ident));
	token = sym->arglist;
	if (token) {
		const char *sep = "";
		int narg = 0;
		putchar('(');
		for (; !eof_token(token); token = token->next) {
			if (token_type(token) == TOKEN_ARG_COUNT)
				continue;
			if (is_VA_ARGS_token(token))
				printf("%s...", sep);
			else
				printf("%s%s", sep, show_token(token));
			args[narg++] = token;
			sep = ",";
		}
		putchar(')');
	}

	dump_macro_tokens(sym->expansion, args, true);
	putchar('\n');
}

void dump_macro_definitions(void)
{
	struct ident *name;

	FOR_EACH_PTR(macros, name) {
		struct symbol *sym = lookup_macro(name);
		if (sym)
			dump_macro(sym);
	} END_FOR_EACH_PTR(name);
}
