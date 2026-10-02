#include <stdarg.h>

static void v1(...)
{
}

static int sum_two(...)
{
	va_list ap, copy;
	int a, b;

	va_start(ap);
	va_copy(copy, ap);
	a = va_arg(ap, int);
	b = va_arg(ap, int);
	va_end(copy);
	va_end(ap);
	return a + b;
}

static int sum_after_n(int n, ...)
{
	va_list ap;
	int a;

	va_start(ap, n);
	va_end(ap);
	va_start(ap, undeclared_ignored_token, 1, 2);
	a = va_arg(ap, int);
	va_end(ap);
	return n + a;
}

static int test(void)
{
	int (*fp)(...) = sum_two;

	v1();
	v1(1);
	v1(1, 2, "three");
	return fp(10, 20) + sum_after_n(5, 7);
}

/*
 * check-name: c23-variadic-no-named
 * check-command: sparse -std=c23 -pedantic $file
 */
