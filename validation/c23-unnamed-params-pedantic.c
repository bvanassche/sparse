static void f1(int)
{
}

/*
 * check-name: c23-unnamed-params-pedantic
 * check-command: sparse -std=c11 $file
 *
 * check-error-start
c23-unnamed-params-pedantic.c:1:16: error: omitting parameter name in function definition is a C23 feature
 * check-error-end
 */
