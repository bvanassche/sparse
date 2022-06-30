/*
 * Example trivial client program that uses the sparse library
 * to tokenize, preprocess and parse a C file, and prints out
 * the results.
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
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"
#include "allocate.h"
#include "token.h"
#include "parse.h"
#include "symbol.h"
#include "expression.h"
#include "linearize.h"

static int context_increase(struct basic_block *bb, int entry)
{
	int sum = 0;
	struct instruction *insn;

	FOR_EACH_PTR(bb->insns, insn) {
		int val;
		if (!insn->bb)
			continue;
		if (insn->opcode != OP_CONTEXT)
			continue;
		val = insn->increment;
		if (insn->check) {
			int current = sum + entry;
			if (!val) {
				if (!current)
					continue;
			} else if (current >= val)
				continue;
			if (Wcontext)
				warning(insn->pos, "context check failure");
			continue;
		}
		sum += val;
		if (entry + sum < 0)
			return sum;
	} END_FOR_EACH_PTR(insn);
	return sum;
}

static int imbalance(struct entrypoint *ep, struct basic_block *bb, int entry, int exit, const char *why)
{
	if (Wcontext) {
		struct symbol *sym = ep->name;
		warning(bb->pos, "context imbalance in '%s' - %s", show_ident(sym->ident), why);
	}
	return -1;
}

struct cond_context_state {
	int cond_out;
	bool saw_base_out;
	bool saw_cond_out;
	bool all_zero;
	bool all_nonzero;
	struct basic_block *any_exit_bb;
};

static bool is_pure_return_bb(struct basic_block *bb)
{
	struct instruction *insn, *last = NULL;

	if (!bb)
		return false;
	FOR_EACH_PTR(bb->insns, insn) {
		if (!insn->bb)
			continue;
		if (insn->opcode == OP_CONTEXT || insn->opcode == OP_CALL ||
		    insn->opcode == OP_INLINED_CALL || insn->opcode == OP_ASM)
			return false;
		last = insn;
	} END_FOR_EACH_PTR(insn);
	return last && last->opcode == OP_RET;
}

static int known_cond_on_edge(pseudo_t cond, struct basic_block *from, struct basic_block *to)
{
	int hops;

	for (hops = 0; from && to && hops < 4; hops++) {
		struct instruction *br = last_instruction(from->insns);
		if (br && br->opcode == OP_CBR && br->cond == cond &&
		    br->bb_true != br->bb_false) {
			if (to == br->bb_true)
				return 1;
			if (to == br->bb_false)
				return 0;
		}
		if (ptr_list_size((struct ptr_list *)from->parents) != 1)
			break;
		to = from;
		from = first_basic_block(from->parents);
		if (from == to)
			break;
	}
	return -1;
}

static int return_truth_on_edge(pseudo_t val, struct basic_block *from,
				struct basic_block *ret_bb, int depth)
{
	struct instruction *def;
	int known;

	if (!val || val == VOID || depth > 8)
		return -1;
	if (val->type == PSEUDO_VAL)
		return val->value != 0 ? 1 : 0;

	known = known_cond_on_edge(val, from, ret_bb);
	if (known >= 0)
		return known;

	if (val->type != PSEUDO_REG && val->type != PSEUDO_PHI)
		return -1;
	def = val->def;
	if (!def)
		return -1;

	switch (def->opcode) {
	case OP_PHI: {
		struct basic_block *cur = from;
		int hops;
		for (hops = 0; cur && hops < 4; hops++) {
			pseudo_t phi;
			FOR_EACH_PTR(def->phi_list, phi) {
				if (!phi || phi == VOID || !phi->def)
					continue;
				if (phi->def->bb == cur)
					return return_truth_on_edge(phi->def->phi_src,
								    cur, ret_bb, depth + 1);
			} END_FOR_EACH_PTR(phi);
			if (ptr_list_size((struct ptr_list *)cur->parents) != 1)
				break;
			cur = first_basic_block(cur->parents);
		}
		return -1;
	}
	case OP_PHISOURCE:
		return return_truth_on_edge(def->phi_src, def->bb, ret_bb, depth + 1);
	case OP_ZEXT:
	case OP_SEXT:
		if (def->orig_type && def->size >= def->orig_type->bit_size)
			return return_truth_on_edge(def->src, from, ret_bb, depth + 1);
		return -1;
	case OP_SET_EQ:
		if (def->src2 && def->src2->type == PSEUDO_VAL && def->src2->value == 0) {
			int t = return_truth_on_edge(def->src1, from, ret_bb, depth + 1);
			return t >= 0 ? !t : -1;
		}
		return -1;
	case OP_SET_NE:
		if (def->src2 && def->src2->type == PSEUDO_VAL && def->src2->value == 0)
			return return_truth_on_edge(def->src1, from, ret_bb, depth + 1);
		return -1;
	case OP_SEL: {
		int ct = return_truth_on_edge(def->cond, from, ret_bb, depth + 1);
		int tt, ft;
		if (ct == 1)
			return return_truth_on_edge(def->src2, from, ret_bb, depth + 1);
		if (ct == 0)
			return return_truth_on_edge(def->src3, from, ret_bb, depth + 1);
		tt = return_truth_on_edge(def->src2, from, ret_bb, depth + 1);
		ft = return_truth_on_edge(def->src3, from, ret_bb, depth + 1);
		return (tt >= 0 && tt == ft) ? tt : -1;
	}
	default:
		return -1;
	}
}

static int check_exit_edge(struct entrypoint *ep, struct basic_block *from,
			   struct basic_block *ret_bb, struct instruction *ret_insn,
			   int entry, int exit, struct cond_context_state *cs)
{
	struct basic_block *diag_bb = from ? from : ret_bb;
	int truth;

	if (!cs || !cs->cond_out)
		return entry != exit ? imbalance(ep, ret_bb, entry, exit, "wrong count at exit") : 0;

	if (!ret_insn->src || ret_insn->src == VOID)
		return imbalance(ep, diag_bb, entry, exit + cs->cond_out, "wrong count at exit");

	truth = return_truth_on_edge(ret_insn->src, from, ret_bb, 0);
	if (!cs->any_exit_bb)
		cs->any_exit_bb = diag_bb;
	if (entry == exit)
		cs->saw_base_out = true;
	if (entry == exit + cs->cond_out)
		cs->saw_cond_out = true;
	if (truth != 0)
		cs->all_zero = false;
	if (truth != 1)
		cs->all_nonzero = false;

	if (truth == 0)
		return entry != exit ? imbalance(ep, diag_bb, entry, exit, "wrong count at exit") : 0;
	if (truth == 1)
		return entry != exit + cs->cond_out ?
			imbalance(ep, diag_bb, entry, exit + cs->cond_out, "wrong count at exit") : 0;
	if (entry != exit && entry != exit + cs->cond_out)
		return imbalance(ep, diag_bb, entry, exit, "wrong count at exit");
	return 0;
}

static int check_bb_context(struct entrypoint *ep, struct basic_block *bb,
			    int entry, int exit, struct cond_context_state *cs);

static int check_children(struct entrypoint *ep, struct basic_block *bb,
			  int entry, int exit, struct cond_context_state *cs)
{
	struct instruction *insn;
	struct basic_block *child;

	insn = last_instruction(bb->insns);
	if (!insn)
		return 0;
	if (insn->opcode == OP_RET)
		return check_exit_edge(ep, NULL, bb, insn, entry, exit, cs);

	FOR_EACH_PTR(bb->children, child) {
		if (cs && cs->cond_out && is_pure_return_bb(child)) {
			if (check_exit_edge(ep, bb, child, last_instruction(child->insns),
					    entry, exit, cs))
				return -1;
			continue;
		}
		if (check_bb_context(ep, child, entry, exit, cs))
			return -1;
	} END_FOR_EACH_PTR(child);
	return 0;
}

static int check_bb_context(struct entrypoint *ep, struct basic_block *bb,
			    int entry, int exit, struct cond_context_state *cs)
{
	if (!bb)
		return 0;
	if (bb->context == entry)
		return 0;

	/* Now that's not good.. */
	if (bb->context >= 0)
		return imbalance(ep, bb, entry, bb->context, "different lock contexts for basic block");

	bb->context = entry;
	entry += context_increase(bb, entry);
	if (entry < 0)
		return imbalance(ep, bb, entry, exit, "unexpected unlock");

	return check_children(ep, bb, entry, exit, cs);
}

static void check_cast_instruction(struct instruction *insn)
{
	struct symbol *orig_type = insn->orig_type;
	if (orig_type) {
		int old = orig_type->bit_size;
		int new = insn->size;
		int oldsigned = (orig_type->ctype.modifiers & MOD_SIGNED) != 0;
		int newsigned = insn->opcode == OP_SEXT;

		if (new > old) {
			if (oldsigned == newsigned)
				return;
			if (newsigned)
				return;
			warning(insn->pos, "cast loses sign");
			return;
		}
		if (new < old) {
			warning(insn->pos, "cast drops bits");
			return;
		}
		if (oldsigned == newsigned) {
			warning(insn->pos, "cast wasn't removed");
			return;
		}
		warning(insn->pos, "cast changes sign");
	}
}

static void check_range_instruction(struct instruction *insn)
{
	warning(insn->pos, "value out of range");
}

static void check_byte_count(struct instruction *insn, pseudo_t count)
{
	if (!count)
		return;
	if (count->type == PSEUDO_VAL) {
		unsigned long long val = count->value;
		if (Wmemcpy_max_count && val > fmemcpy_max_count)
			warning(insn->pos, "%s with byte count of %llu",
				show_ident(insn->func->sym->ident), val);
		return;
	}
	/* OK, we could try to do the range analysis here */
}

static void check_memset(struct instruction *insn)
{
	check_byte_count(insn, ptr_list_nth(insn->arguments, 2));
}

#define check_memcpy check_memset
#define check_ctu check_memset
#define check_cfu check_memset

struct checkfn {
	struct ident *id;
	void (*check)(struct instruction *insn);
};

static void check_call_instruction(struct instruction *insn)
{
	pseudo_t fn = insn->func;
	struct ident *ident;
	static const struct checkfn check_fn[] = {
		{ &memset_ident, check_memset },
		{ &memcpy_ident, check_memcpy },
		{ &copy_to_user_ident, check_ctu },
		{ &copy_from_user_ident, check_cfu },
	};
	int i;

	if (fn->type != PSEUDO_SYM)
		return;
	ident = fn->sym->ident;
	if (!ident)
		return;
	for (i = 0; i < ARRAY_SIZE(check_fn); i++) {
		if (check_fn[i].id != ident)
			continue;
		check_fn[i].check(insn);
		break;
	}
}

static void check_one_instruction(struct instruction *insn)
{
	switch (insn->opcode) {
	case OP_SEXT: case OP_ZEXT:
	case OP_TRUNC:
		if (verbose)
			check_cast_instruction(insn);
		break;
	case OP_RANGE:
		check_range_instruction(insn);
		break;
	case OP_CALL:
		check_call_instruction(insn);
		break;
	default:
		break;
	}
}

static void check_bb_instructions(struct basic_block *bb)
{
	struct instruction *insn;
	FOR_EACH_PTR(bb->insns, insn) {
		if (!insn->bb)
			continue;
		check_one_instruction(insn);
	} END_FOR_EACH_PTR(insn);
}

static void check_instructions(struct entrypoint *ep)
{
	struct basic_block *bb;
	FOR_EACH_PTR(ep->bbs, bb) {
		bb->context = -1;
		check_bb_instructions(bb);
	} END_FOR_EACH_PTR(bb);
}

static void check_context(struct entrypoint *ep)
{
	struct symbol *sym = ep->name;
	struct context *context;
	unsigned int in_context = 0, out_context = 0;

	if (Wuninitialized && verbose && ep->entry->bb->needs) {
		pseudo_t pseudo;
		FOR_EACH_PTR(ep->entry->bb->needs, pseudo) {
			if (pseudo->type != PSEUDO_ARG)
				warning(sym->pos, "%s: possible uninitialized variable (%s)",
					show_ident(sym->ident), show_pseudo(pseudo));
		} END_FOR_EACH_PTR(pseudo);
	}

	check_instructions(ep);

	struct cond_context_state cs = {
		.all_zero = true,
		.all_nonzero = true,
	};

	FOR_EACH_PTR(sym->ctype.contexts, context) {
		int in = context->in;
		int out = context->out;

		if (in > 0)
			in_context += in;
		if (out > 0)
			out_context += out;
		else if (out < 0)
			cs.cond_out += -out;
	} END_FOR_EACH_PTR(context);
	if (!check_bb_context(ep, ep->entry->bb, in_context, out_context, &cs) &&
	    cs.cond_out > 0 && cs.any_exit_bb) {
		if ((!cs.all_zero && !cs.saw_cond_out) ||
		    (!cs.all_nonzero && !cs.saw_base_out))
			imbalance(ep, cs.any_exit_bb, in_context,
				  out_context + cs.cond_out, "wrong count at exit");
	}
}

/* list_compound_symbol - symbol info for arrays, structures, unions */
static void list_compound_symbol(struct symbol *sym)
{
	struct symbol *base;

	/* Only show symbols that have a positive size */
	if (sym->bit_size <= 0)
		return;
	if (!sym->ctype.base_type)
		return;
	/* Don't show unnamed types */
	if (!sym->ident)
		return;

	if (sym->type == SYM_NODE)
		base = sym->ctype.base_type;
	else
		base = sym;
	switch (base->type) {
	case SYM_STRUCT: case SYM_UNION: case SYM_ARRAY:
		break;
	default:
		return;
	}

	info(sym->pos, "%s: compound size %u, alignment %lu",
		show_typename(sym),
		bits_to_bytes(sym->bit_size),
		sym->ctype.alignment);
}

static void check_symbols(struct symbol_list *list)
{
	struct symbol *sym;

	FOR_EACH_PTR(list, sym) {
		struct entrypoint *ep;

		expand_symbol(sym);
		ep = linearize_symbol(sym);
		if (ep && ep->entry) {
			if (dbg_entry)
				show_entry(ep);

			check_context(ep);
		}
		if (dbg_compound)
			list_compound_symbol(sym);
	} END_FOR_EACH_PTR(sym);

	if (Wsparse_error && die_if_error)
		exit(1);
}

int main(int argc, char **argv)
{
	struct string_list *filelist = NULL;
	char *file;

	// by default ignore -o <file>
	do_output = 0;

	// Expand, linearize and show it.
	check_symbols(sparse_initialize(argc, argv, &filelist));
	FOR_EACH_PTR(filelist, file) {
		check_symbols(sparse(file));
	} END_FOR_EACH_PTR(file);

	report_stats();
	return 0;
}
