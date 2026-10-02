static int test(void)
{
	alignas(8) int x = 0;
	int a = alignof(int);
	int b = alignof(x);
	int c = _Alignof(x);
	int d = __alignof__(x);
	int e = __alignof(x);
	return a + b + c + d + e;
}

/*
 * check-name: c23-alignof-pedantic
 * check-command: sparse -std=c23 -pedantic $file
 *
 * check-error-start
c23-alignof-pedantic.c:5:17: warning: ISO C does not allow 'alignof' on an expression
c23-alignof-pedantic.c:6:17: warning: ISO C does not allow '_Alignof' on an expression
 * check-error-end
 */
