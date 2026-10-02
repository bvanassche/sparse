static void f_implicit(const)
{
}

static void f_void1(void, int)
{
}

static void f_void2(int, void)
{
}

static void f_cvoid(const void)
{
}

/*
 * check-name: c23-unnamed-params-error
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-unnamed-params-error.c:1:24: error: missing type declaration for parameter
c23-unnamed-params-error.c:5:21: error: 'void' must be the only parameter
c23-unnamed-params-error.c:9:26: error: 'void' must be the only parameter
c23-unnamed-params-error.c:13:21: error: 'void' must be the only parameter
 * check-error-end
 */
