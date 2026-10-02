static int (bool);
static int (true);
static int (false);
typedef int bool;

static void bad_lvalue(void)
{
	true = false;
}

/*
 * check-name: c23-bool-error
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
c23-bool-error.c:1:12: error: Trying to use reserved word 'bool' as identifier
c23-bool-error.c:2:12: error: Trying to use reserved word 'true' as identifier
c23-bool-error.c:3:12: error: Trying to use reserved word 'false' as identifier
c23-bool-error.c:4:13: error: two or more data types in declaration specifiers
c23-bool-error.c:4:13: error: Trying to use reserved word 'bool' as identifier
c23-bool-error.c:8:14: error: not an lvalue
 * check-error-end
 */
