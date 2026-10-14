static void test_volatile_stores(volatile int *p)
{
	*p = 1;
	*p = 2;
}

static void test_asm_mem(int *p)
{
	int x = 42;
	asm volatile("" : : "m"(x));
}

struct mixed_bf {
	int a:20;
	int b:20;
};

static struct mixed_bf ssa_mixed_load(int c, struct mixed_bf x, struct mixed_bf y, int *out)
{
	struct mixed_bf s;
	if (c)
		s = x;
	else
		s = y;
	struct mixed_bf copy = s;
	*out = s.a;
	return copy;
}

extern int glob;
extern void ext_call(void);

static void dom_store_order(int c)
{
	glob = 1;
	if (c)
		ext_call();
	else
		glob = 3;
	glob = 2;
}

/*
 * check-name: volatile-dse-and-cfg
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-excludes: VOID
 * check-output-contains: store\\.32.*\\$1 -> 0\\[%arg1\\]
 * check-output-contains: store\\.32.*\\$2 -> 0\\[%arg1\\]
 * check-output-contains: store\\.32.*\\$42 -> 0\\[x\\]
 * check-output-contains: store\\.32.*\\$1 -> 0\\[glob\\]
 */
