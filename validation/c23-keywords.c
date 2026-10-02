static_assert(1);
static_assert(1, "file scope");

struct s {
	static_assert(1);
	static_assert(1, "struct scope");
	alignas(16) int a;
};

static_assert(alignof(struct s) == 16);

thread_local int tl;
alignas(double) static int aligned_dbl;

static int test(void)
{
	static_assert(1);
	static_assert(1, "func scope");
	alignas(8) int x = 0;
	static thread_local int local_tl = 3;
	extern thread_local int tl;
	static_assert(alignof(int) <= 8);
	static_assert(alignof(double) <= 16);
	return x + tl + local_tl + aligned_dbl;
}

/*
 * check-name: c23-keywords
 * check-command: sparse -std=c23 -pedantic -Wno-decl $file
 */
