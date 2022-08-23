#ifndef SIMPLIFY_H
#define SIMPLIFY_H

#include "linearize.h"

int simplify_instruction(struct instruction *insn);
int early_simplify_preop(struct expression *expr);

int replace_with_pseudo(struct instruction *insn, pseudo_t pseudo);

#endif
