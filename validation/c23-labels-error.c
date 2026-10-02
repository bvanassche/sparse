static void test_dup(void)
{
dup:
	;
dup:
	;
}

static void test_if_decl(int c)
{
	if (c)
		int x = 1;
}

static void test_if_label_decl(int c)
{
	if (c)
	lbl1:
		int y = 2;
}

static void test_if_label_end(int c)
{
	if (c)
	lbl2:
}

static void test_outside_switch(void)
{
	case 1:
		;
	default:
		;
}

/*
 * check-name: c23-labels-error
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
c23-labels-error.c:5:1: error: label 'dup' redefined
c23-labels-error.c:12:17: error: declaration is not a statement
c23-labels-error.c:19:17: error: declaration is not a statement
c23-labels-error.c:26:1: error: statement expected after label
c23-labels-error.c:30:9: error: not in switch scope
c23-labels-error.c:32:9: error: not in switch scope
 * check-error-end
 */
