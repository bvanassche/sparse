void *f(void);
void g(void);
int h(void);

void test_cond_void_ir(int c, void *p)
{
	c ? f() : g();
	c ? g() : h();
	c ? 1 : g();
	p ?: g();
	1 ?: g();
	0 ?: g();
}

/*
 * check-name: cond-one-void-linearize
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-excludes: VOID
 * check-output-excludes: phi\\.
 * check-output-excludes: phisrc\\.
 * check-output-excludes: select\\.
 */
