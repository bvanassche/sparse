static const int *p = (static int[]){ 1, 2, 3 };
static const int *cp = (constexpr int[]){ 4, 5, 6 };

static int test(void)
{
	static int *lp = (static int[]){ 7, 8, 9 };
	register int rv = (register int){ 12 };
	int *tp = (static thread_local int[]){ 14 };
	const int *lcp = &(static constexpr int){ 99 };
	int auto_ce = (constexpr int){ 42 };

	return lp[0] + rv + tp[0] + *lcp + auto_ce + p[0] + cp[0];
}

static_assert(sizeof((static int[]){ 1, 2, 3 }) == 3 * sizeof(int),
	      "sizeof static compound literal");
static_assert(sizeof((constexpr int[]){ 1, 2, 3, 4 }) == 4 * sizeof(int),
	      "sizeof constexpr compound literal");
static_assert(sizeof((thread_local int[]){ 14, 15 }) == 2 * sizeof(int),
	      "sizeof file-scope thread_local compound literal");
static_assert((constexpr int){ 42 } == 42,
	      "constexpr scalar compound literal in ICE");

/*
 * check-name: c23-compound-literal-storage
 * check-command: sparse -std=c23 -pedantic $file
 */
