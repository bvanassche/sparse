int f();
int f(void)
{
	return 0;
}

int g(void);
int g()
{
	return 1;
}

static int h()
{
	return 2;
}

static int test(void)
{
	int (*fp)() = f;
	int (*fp2)(void) = h;

	return fp() + fp2() + g();
}

/*
 * check-name: c23-empty-parameters
 * check-command: sparse -std=c23 -Wstrict-prototypes -pedantic $file
 */
