#define __cleanup(f) __attribute__((cleanup(f)))
#define __context_lock(x) __attribute__((context(0, 1)))
#define __context_unlock(x) __attribute__((context(1, 0)))

static void lock(int *x) __context_lock(x)
{
	__context__(1);
}

static void unlock(int **x) __context_unlock(x)
{
	__context__(-1);
}

static void test_normal_exit(int *m)
{
	int *guard __cleanup(unlock) = (lock(m), m);
}

static int test_return(int *m, int cond)
{
	int *guard __cleanup(unlock) = (lock(m), m);
	if (cond)
		return 1;
	return 0;
}

static int test_nested_return(int *m1, int *m2, int cond)
{
	int *g1 __cleanup(unlock) = (lock(m1), m1);
	{
		int *g2 __cleanup(unlock) = (lock(m2), m2);
		if (cond)
			return -1;
	}
	return 0;
}

static void test_loop_break(int *m, int n)
{
	for (int i = 0; i < n; i++) {
		int *guard __cleanup(unlock) = (lock(m), m);
		if (i == 2)
			break;
	}
}

static void test_loop_continue(int *m, int n)
{
	for (int i = 0; i < n; i++) {
		int *guard __cleanup(unlock) = (lock(m), m);
		if (i == 2)
			continue;
	}
}

static void test_backward_goto(int *m, int cond)
{
again:
	{
		int *guard __cleanup(unlock) = (lock(m), m);
		if (cond) {
			cond = 0;
			goto again;
		}
	}
}

static void test_forward_goto(int *m, int cond)
{
	{
		int *guard __cleanup(unlock) = (lock(m), m);
		if (cond)
			goto out;
	}
out:
	return;
}

static void test_loop_header_break(int *m, int n)
{
	for (int *guard __cleanup(unlock) = (lock(m), m); n > 0; n--) {
		if (n == 2)
			break;
	}
}

static void test_switch_break(int *m, int val)
{
	switch (val) {
	case 1: {
		int *guard __cleanup(unlock) = (lock(m), m);
		break;
	}
	default:
		break;
	}
}

static void test_switch_case_decl(int *m, int val)
{
	switch (val) {
	default:
		break;
	case 1: ;
		int *guard __cleanup(unlock) = (lock(m), m);
		break;
	}
}

static void test_computed_goto_const(int *m)
{
	{
		int *guard __cleanup(unlock) = (lock(m), m);
		goto *&&out;
	}
out:
	return;
}

static void test_computed_goto_dyn(int *m, int cond)
{
	void *target = cond ? &&out1 : &&out2;
	{
		int *guard __cleanup(unlock) = (lock(m), m);
		goto *target;
	}
out1:
	return;
out2:
	return;
}

static void test_multi_declarator(int *m1, int *m2)
{
	int __cleanup(unlock) *g1 = (lock(m1), m1), *g2 = (lock(m2), m2);
}

static void test_stmt_expr_init_backward_goto(int *m, int cond)
{
again:
	int *guard __cleanup(unlock) = ({ int *p = (lock(m), m); p; });
	if (cond) {
		cond = 0;
		goto again;
	}
}

static void test_warn_imbalance(int *m)
{
	lock(m);
}

/*
 * check-name: cleanup attribute in context analysis
 * check-command: sparse -Wcontext $file
 *
 * check-error-start
cleanup-context.c:149:13: warning: context imbalance in 'test_warn_imbalance' - wrong count at exit
 * check-error-end
 */
