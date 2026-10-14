/* 1. Statement expression with return inside if condition */
int stmt_expr_cond(void)
{
	if (({ if (0) return 0; 1; }))
		return 1;
	return 0;
}

/* 2. Logical expression with unreachable RHS after return */
int logical_after_return(int x)
{
	return x && ({ return 2; 1; });
}

/* 3. asm "+r" read-write register and "m" memory operand preserve stores */
int asm_rw_and_mem(void)
{
	int x = 42;
	int y = 99;
	asm("" : "+r"(x) : "m"(y));
	return x;
}

/* 4. Switch on signed char with negative case label */
int switch_signed_char(signed char c)
{
	switch (c) {
	case -1:
		return 1;
	default:
		return 0;
	}
}

/*
 * check-name: control-flow-fixes
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-excludes: VOID
 * check-output-contains: store\\.32.*\\$99 -> 0\\[y\\]
 * check-output-contains: in:.*"+r" (\\$42)
 * check-output-contains: switch\\.8.*-1 ->
 */
