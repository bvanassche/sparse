typedef int T_shadow;

static int test_label_decl(void)
{
	goto lbl;
lbl:
lbl_chain:
	int x = 42;
	if (!x)
		goto lbl_chain;
	return x;
}

static int test_case_decl(int c)
{
	switch (c) {
	case 1:
		int a = 10;
		return a;
	default:
		int b = 20;
		return b;
	}
}

static void test_label_end(int c)
{
	if (c == 1)
		goto end1;
	if (c == 2)
		goto end2;
end1:
end2:
}

static void test_case_end(int c)
{
	switch (c) {
	case 0:
	case 1:
	}
	switch (c) {
	default:
	}
}

static void test_label_static_assert(void)
{
	goto lbl2;
lbl2:
	static_assert(1, "");
}

static int test_typedef_shadow_label(int c)
{
	if (c)
		goto T_shadow;
	return 0;
T_shadow:
	int v = 1;
	return v;
}

/*
 * check-name: c23-labels
 * check-command: sparse -std=c23 -pedantic -Wno-decl $file
 */
