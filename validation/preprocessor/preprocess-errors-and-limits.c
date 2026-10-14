#if defined()
#endif
#if defined(FOO
#endif
#if defined
#endif
#if defined(
#endif
#include "."
static const char *s = "\x";

/* 300-character identifier */
static int aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa = 42;

static int get_val(void)
{
	return aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa;
}

/*
 * check-name: preprocess-errors-and-limits
 * check-command: sparse $file
 * check-exit-value: 1
 *
 * check-error-start
preprocessor/preprocess-errors-and-limits.c:10:27: warning: \x used with no following hex digits
preprocessor/preprocess-errors-and-limits.c:1:13: error: expected preprocessor identifier
preprocessor/preprocess-errors-and-limits.c:3:13: error: missing ')' after "defined"
preprocessor/preprocess-errors-and-limits.c:5:5: error: expected identifier after "defined"
preprocessor/preprocess-errors-and-limits.c:7:5: error: expected identifier after "defined"
preprocessor/preprocess-errors-and-limits.c:9:10: error: unable to open '.'
 * check-error-end
 */
