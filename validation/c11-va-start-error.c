#include <stdarg.h>

static void f(int n, ...)
{
	va_list ap;
	(void)n;
	va_start(ap);
	__builtin_va_start(ap);
}

/*
 * check-name: c11-va-start-error
 * check-command: sparse -std=c11 $file
 *
 * check-error-start
c11-va-start-error.c:7:9: error: macro "va_start" requires 2 arguments, but only 1 given
c11-va-start-error.c:7:9: error: undefined identifier 'va_start'
c11-va-start-error.c:8:27: error: not enough arguments for __builtin_va_start
 * check-error-end
 */
