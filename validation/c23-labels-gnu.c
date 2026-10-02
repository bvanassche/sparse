static int test_gnu_case_range(int c)
{
	switch (c) {
	case 1 ... 5:
		int a = 3;
		return a;
	case 6 ... 9:
	}
	return 0;
}

/*
 * check-name: c23-labels-gnu
 * check-command: sparse -std=gnu23 -Wno-decl $file
 */
