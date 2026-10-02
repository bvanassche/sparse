static void test_label_decl(void)
{
	goto lbl;
lbl:
	int x = 42;
}

/*
 * check-name: c23-labels-pedantic
 * check-command: sparse -std=c11 -pedantic -Wno-decl $file
 *
 * check-error-start
c23-labels-pedantic.c:5:9: warning: a label can only be part of a statement and a declaration is not a statement
 * check-error-end
 */
