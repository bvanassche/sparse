#include <stdarg.h>

int first_arg(...);
int first_arg(...)
{
	va_list ap;
	int v;

	va_start(ap);
	v = va_arg(ap, int);
	va_end(ap);
	return v;
}

/*
 * check-name: linear-c23-va-start
 * check-command: test-linearize -m64 -std=c23 -Wuninitialized $file
 *
 * check-output-ignore
 * check-output-excludes: __builtin_va_start
 * check-output-excludes: __builtin_c23_va_start
 */
