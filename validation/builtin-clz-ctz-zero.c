static int ctz_zero = __builtin_ctz(0);
static int clz_zero = __builtin_clz(0);
static int ctzll_zero = __builtin_ctzll(0);
static int clzll_zero = __builtin_clzll(0);

/*
 * check-name: __builtin_clz and __builtin_ctz with zero
 * check-command: sparse -Wno-decl $file
 * check-error-start
 * check-error-end
 */
