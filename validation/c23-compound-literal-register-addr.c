static void bad_reg_addr(void)
{
	int *p = &(register int){ 1 };
	int *q = (register int[]){ 1, 2 };
	(void)p;
	(void)q;
}

/*
 * check-name: c23-compound-literal-register-addr
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-compound-literal-register-addr.c:3:20: warning: taking address of 'register' compound literal
c23-compound-literal-register-addr.c:4:19: warning: taking address of 'register' compound literal
 * check-error-end
 */
