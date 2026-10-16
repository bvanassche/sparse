#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "target.h"
#include "lib.h"
#include "allocate.h"
#include "token.h"
#include "expression.h"
#include "char.h"

static const char *parse_ucn(const char *p, const char *end, int digits,
			     unsigned *cp, struct position pos)
{
	unsigned val = 0;
	int i;

	for (i = 0; i < digits; i++) {
		unsigned d;
		if (p >= end || (d = hexval(*p)) > 15) {
			sparse_error(pos, "incomplete universal character name");
			*cp = 0;
			return p;
		}
		val = (val << 4) + d;
		p++;
	}
	if ((val >= 0xd800 && val <= 0xdfff) || val > 0x10ffff) {
		sparse_error(pos, "invalid universal character name");
		val = 0;
	}
	*cp = val;
	return p;
}

static int encode_utf8(unsigned cp, char out[4])
{
	if (cp <= 0x7f) {
		out[0] = cp;
		return 1;
	}
	if (cp <= 0x7ff) {
		out[0] = 0xc0 | (cp >> 6);
		out[1] = 0x80 | (cp & 0x3f);
		return 2;
	}
	if (cp <= 0xffff) {
		out[0] = 0xe0 | (cp >> 12);
		out[1] = 0x80 | ((cp >> 6) & 0x3f);
		out[2] = 0x80 | (cp & 0x3f);
		return 3;
	}
	out[0] = 0xf0 | (cp >> 18);
	out[1] = 0x80 | ((cp >> 12) & 0x3f);
	out[2] = 0x80 | ((cp >> 6) & 0x3f);
	out[3] = 0x80 | (cp & 0x3f);
	return 4;
}

static const char *parse_escape(const char *p, unsigned *val, const char *end, int bits, struct position pos)
{
	unsigned c = (unsigned char)*p++;
	unsigned d;
	if (c != '\\') {
		*val = c;
		return p;
	}

	if (p < end)
		c = (unsigned char)*p++;
	switch (c) {
	case 'a': c = '\a'; break;
	case 'b': c = '\b'; break;
	case 't': c = '\t'; break;
	case 'n': c = '\n'; break;
	case 'v': c = '\v'; break;
	case 'f': c = '\f'; break;
	case 'r': c = '\r'; break;
	case 'e': c = '\e'; break;
	case 'u':
	case 'U':
		p = parse_ucn(p, end, c == 'u' ? 4 : 8, &c, pos);
		*val = c;
		return p;
	case 'x': {
		unsigned mask = -(1U << (bits - 4));
		for (c = 0; p < end; c = (c << 4) + d) {
			d = hexval(*p);
			if (d > 16)
				break;
			p++;
			if (c & mask) {
				warning(pos,
					"hex escape sequence out of range");
				mask = 0;
			}
		}
		break;
	}
	case '0'...'7': {
		if (p + 2 < end)
			end = p + 2;
		c -= '0';
		while (p < end && (d = *p - '0') < 8) {
			c = (c << 3) + d;
			p++;
		}
		if ((c & 0400) && bits < 9)
			warning(pos,
				"octal escape sequence out of range");
		break;
	}
	default:	/* everything else is left as is */
		warning(pos, "unknown escape sequence: '\\%c'", c);
		break;
	case '\\':
	case '\'':
	case '"':
	case '?':
		break;	/* those are legal, so no warnings */
	}
	*val = c & ~((~0U << (bits - 1)) << 1);
	return p;
}

void get_char_constant(struct token *token, unsigned long long *val)
{
	const char *p = token->embedded, *end;
	unsigned v;
	int type = token_type(token);
	switch (type) {
	case TOKEN_CHAR:
	case TOKEN_WIDE_CHAR:
	case TOKEN_UTF8_CHAR:
		p = token->string->data;
		end = p + token->string->length - 1;
		if (end == p) {
			sparse_error(token->pos, "empty character constant");
			*val = 0;
			return;
		}
		break;
	case TOKEN_CHAR_EMBEDDED_0 ... TOKEN_CHAR_EMBEDDED_3:
		end = p + type - TOKEN_CHAR;
		break;
	case TOKEN_WIDE_CHAR_EMBEDDED_0 ... TOKEN_WIDE_CHAR_EMBEDDED_3:
		end = p + type - TOKEN_WIDE_CHAR;
		break;
	default:
		end = p + type - TOKEN_UTF8_CHAR;
		break;
	}
	p = parse_escape(p, &v, end,
			 (type >= TOKEN_WIDE_CHAR &&
			  type <= TOKEN_WIDE_CHAR_EMBEDDED_3) ?
				 wchar_ctype->bit_size :
				 bits_in_char,
			 token->pos);
	if (type >= TOKEN_UTF8_CHAR && type <= TOKEN_UTF8_CHAR_EMBEDDED_3) {
		if (p != end || v > 0xff ||
		    (token->embedded[0] == '\\' &&
		     (token->embedded[1] == 'u' || token->embedded[1] == 'U') &&
		     v > 0x7f) ||
		    (type == TOKEN_UTF8_CHAR && token->string->data[0] == '\\' &&
		     (token->string->data[1] == 'u' ||
		      token->string->data[1] == 'U') &&
		     v > 0x7f))
			sparse_error(token->pos,
				     "multi-character UTF-8 character constant");
	} else if (p != end) {
		warning(token->pos,
			"multi-character character constant");
	}
	*val = v;
}

static void append_string_byte(char **buf, size_t *cap, size_t *len, char ch)
{
	if (*len >= INT_MAX - sizeof(struct string))
		die("string too large");
	if (*len + 1 > *cap) {
		size_t new_cap = *cap ? *cap * 2 : 4096;
		if (new_cap < *len + 1)
			new_cap = *len + 1;
		*buf = realloc(*buf, new_cap);
		if (!*buf)
			die("out of memory");
		*cap = new_cap;
	}
	(*buf)[(*len)++] = ch;
}

struct token *get_string_constant(struct token *token, struct expression *expr)
{
	struct string *string = token->string;
	struct token *next = token->next, *done = NULL;
	int stringtype = token_type(token);
	int is_wide = stringtype == TOKEN_WIDE_STRING;
	int is_utf8 = stringtype == TOKEN_UTF8_STRING;
	static char *buffer;
	static size_t buf_size;
	size_t len = 0;
	int bits;
	int esc_count = 0;

	while (!done) {
		switch (token_type(next)) {
		case TOKEN_WIDE_STRING:
			is_wide = 1;
			next = next->next;
			break;
		case TOKEN_UTF8_STRING:
			is_utf8 = 1;
			next = next->next;
			break;
		case TOKEN_STRING:
			next = next->next;
			break;
		default:
			done = next;
		}
	}
	if (is_wide && is_utf8) {
		sparse_error(token->pos,
			     "unsupported non-standard concatenation of string literals");
		is_utf8 = 0;
	}
	bits = is_wide ? wchar_ctype->bit_size: bits_in_char;
	while (token != done) {
		unsigned v;
		const char *p = token->string->data;
		const char *end = p + token->string->length - 1;
		while (p < end) {
			bool is_ucn = (*p == '\\' && p + 1 < end &&
				       (p[1] == 'u' || p[1] == 'U'));
			if (*p == '\\')
				esc_count++;
			p = parse_escape(p, &v, end, bits, token->pos);
			if (is_ucn && !is_wide) {
				char utf8[4];
				int i, n = encode_utf8(v, utf8);
				for (i = 0; i < n; i++)
					append_string_byte(&buffer, &buf_size, &len, utf8[i]);
			} else {
				append_string_byte(&buffer, &buf_size, &len, v);
			}
		}
		token = token->next;
	}

	if (esc_count || len >= string->length) {
		if (string->immutable || len >= string->length)	/* can't cannibalize */
			string = __alloc_string(len + 1);
		string->length = len + 1;
		if (len)
			memcpy(string->data, buffer, len);
		string->data[len] = '\0';
	}
	expr->string = string;
	expr->wide = is_wide;
	expr->utf8 = is_utf8;
	return token;
}
