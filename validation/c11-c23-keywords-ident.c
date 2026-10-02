static int test_c11_idents(void)
{
	int alignas = 1;
	int alignof = 2;
	int static_assert = 3;
	int thread_local = 4;
	int bool = 5;
	int true = 6;
	int false = 7;
	int nullptr = 8;

	return alignas + alignof + static_assert + thread_local +
	       bool + true + false + nullptr;
}

/*
 * check-name: c11-c23-keywords-ident
 * check-command: sparse -std=c11 $file
 */
