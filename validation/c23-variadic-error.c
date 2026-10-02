#include <stdarg.h>

static const va_list const_ap;

static void fixed_fn(int x)
{
	va_list ap;
	(void)x;
	va_start(ap);
}

static void variadic_errors(...)
{
	int not_ap = 0;
	void *void_ptr = nullptr;
	va_list ap;

	__builtin_va_start();
	__builtin_va_start(ap, 1, 2);
	va_start(not_ap);
	va_start(void_ptr);
	va_start(const_ap);
	(void)(va_start(ap) + 1);
}

/*
 * check-name: c23-variadic-error
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-variadic-error.c:9:9: error: 'va_start' used in function with fixed args
c23-variadic-error.c:18:27: error: not enough arguments for __builtin_va_start
c23-variadic-error.c:19:27: error: too many arguments for __builtin_va_start
c23-variadic-error.c:20:9: error: incorrect type in 'va_start'
c23-variadic-error.c:21:9: error: incorrect type in 'va_start'
c23-variadic-error.c:22:9: error: assignment to const expression
c23-variadic-error.c:23:29: error: incompatible types for operation (+):
c23-variadic-error.c:23:29:    void
c23-variadic-error.c:23:29:    int
 * check-error-end
 */
