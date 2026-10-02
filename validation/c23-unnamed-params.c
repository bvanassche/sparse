static void f1(int)
{
}

static void f2(int, double)
{
}

static int f3(int a, double, int b)
{
	return a + b;
}

static void f4(void (*)(int))
{
}

static void f5(int [static 4], const int)
{
}

static inline int inl_unnamed(int a, int, int b)
{
	return a + b;
}

static int bump(int *cnt)
{
	return (*cnt)++;
}

static int test_calls(void)
{
	int arr[4] = { 1, 2, 3, 4 };
	int cnt = 0;

	f1(1);
	f2(2, 3.0);
	f4(f1);
	f5(arr, 7);
	return f3(10, 0.5, 20) + inl_unnamed(1, bump(&cnt), 2) + cnt;
}

/*
 * check-name: c23-unnamed-params
 * check-command: sparse -std=c23 -pedantic $file
 */
