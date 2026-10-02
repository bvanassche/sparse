/*
 * Check that function declarators properly parse parameter lists
 * where parameters have standard C23 attributes.
 */

static void f1([[maybe_unused]] int x)
{
}

static int f2([[maybe_unused]] int a, [[maybe_unused]] int b)
{
	return a;
}

static void (*fp)([[maybe_unused]] int);

static void test_call(void)
{
	f1(10);
	f2(1, 2);
	fp = f1;
	fp(20);
}

/*
 * check-name: C23 parameter attributes
 * check-command: sparse -std=c23 $file
 */
