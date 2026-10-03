void cl1(int *p);
void cl2(int *p);
void cl3(int *p);

/* 1. break and continue inside loop with cleanup variable */
void loop_cleanup(int a, int b)
{
	while (a > 0) {
		int x __attribute__((cleanup(cl1))) = a;
		if (b == 1)
			break;
		if (b == 2)
			continue;
		a--;
	}
}

/* 2. goto out of nested cleanup scopes */
int goto_nested_cleanup(int c)
{
	int outer __attribute__((cleanup(cl1))) = 1;
	{
		int inner __attribute__((cleanup(cl2))) = 2;
		if (c)
			goto out;
	}
out:
	return outer;
}

/* 3. Cleanup declaration inside unbraced switch case runs once, not twice */
void switch_case_cleanup(int v)
{
	switch (v) {
	default:
		break;
	case 1: ;
		int s __attribute__((cleanup(cl3))) = v;
		break;
	}
}

/* 4. Multi-declarator cleanup and statement-expression initializer + backward goto */
void multi_and_stmt_expr_cleanup(int c)
{
again:
	int __attribute__((cleanup(cl3))) a = ({ int t = c; t; }), b = 2;
	if (c) {
		c = 0;
		goto again;
	}
}

/*
 * check-name: cleanup-control-flow
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-pattern(4): call.*cl1
 * check-output-pattern(2): call.*cl2
 * check-output-pattern(5): call.*cl3
 */
