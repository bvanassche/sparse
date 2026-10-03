#define __cleanup(f) __attribute__((cleanup(f)))

static void cl(int *p)
{
	(void)p;
}

static void lock(int *m) __attribute__((context(0, 1)))
{
	(void)m;
	__context__(1);
}

static void unlock(int **m) __attribute__((context(1, 0)))
{
	(void)m;
	__context__(-1);
}

static void bad_param(int x __cleanup(cl))
{
	(void)x;
}

struct bad_struct {
	int member __cleanup(cl);
};

typedef int bad_typedef __cleanup(cl);

static void bad_fn(void) __cleanup(cl);

static void bad_forward_goto(int *m, int c)
{
	if (c)
		goto inside;
	int *guard __cleanup(unlock) = (lock(m), m);
inside:
	return;
}

static void bad_switch_bypass(int *m, int v)
{
	switch (v) {
	case 1: ;
		int *guard __cleanup(unlock) = (lock(m), m);
		break;
	case 2:
		break;
	}
}

static void bad_cgoto_into_scope(void *p)
{
	goto *p;
	{
		int x __cleanup(cl) = 1;
	inner:
		(void)&&inner;
		(void)x;
	}
}

/*
 * check-name: cleanup attribute error diagnostics
 * check-command: sparse -Wcontext $file
 *
 * check-error-start
cleanup-errors.c:20:23: error: cleanup attribute may only be applied to local variables
cleanup-errors.c:26:13: error: cleanup attribute may only be applied to local variables
cleanup-errors.c:29:13: error: cleanup attribute may only be applied to local variables
cleanup-errors.c:31:13: error: cleanup attribute may only be applied to local variables
cleanup-errors.c:36:17: error: jump bypasses initialization of variable 'guard' with cleanup attribute
cleanup-errors.c:48:9: error: jump bypasses initialization of variable 'guard' with cleanup attribute
cleanup-errors.c:55:9: error: jump bypasses initialization of variable 'x' with cleanup attribute
 * check-error-end
 */
