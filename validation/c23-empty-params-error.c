int f();

static int h()
{
	return 0;
}

int mismatch();
int mismatch(int x)
{
	return x;
}

int ident_decl(a, b);
int (*bad_fp)(a, b);

int old_implicit(a, b)
{
	return 0;
}

static void bad_calls(int (*fp)())
{
	f(1);
	h(1, 2);
	fp(3);
}

/*
 * check-name: c23-empty-params-error
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
c23-empty-params-error.c:14:5: error: identifier list not in function definition
c23-empty-params-error.c:15:14: error: identifier list not in definition
c23-empty-params-error.c:17:5: error: old-style function definition is invalid in C23
c23-empty-params-error.c:17:18: error: missing type declaration for parameter 'a'
c23-empty-params-error.c:17:21: error: missing type declaration for parameter 'b'
c23-empty-params-error.c:9:5: error: symbol 'mismatch' redeclared with different type (different argument counts):
c23-empty-params-error.c:9:5:    int extern [addressable] [signed] [toplevel] mismatch( ... )
c23-empty-params-error.c:8:5: note: previously declared as:
c23-empty-params-error.c:8:5:    int extern [addressable] [signed] [toplevel] mismatch( ... )
c23-empty-params-error.c:24:10: error: too many arguments for function f
c23-empty-params-error.c:25:10: error: too many arguments for function h
c23-empty-params-error.c:26:11: error: too many arguments for function fp
 * check-error-end
 */
