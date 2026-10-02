int old_func(a, b)
int a, b;
{
	return a + b;
}

/*
 * check-name: c23-knr-error
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-knr-error.c:2:1: error: old-style function definition is invalid in C23
 * check-error-end
 */
