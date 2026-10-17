void *f(void);
void g(void);
int h(void);
extern void *p;

static void test(int c)
{
	c ? f() : g();
	c ? g() : h();
	c ? 1 : g();
	c ? p : g();
	p ?: g();
	c ? g() : (void)0;		// both sides void: always OK
	(void)(c ? h() : g());
}

static int use_value(int c)
{
	int x = c ? h() : g();		// KO: void value used
	return x;
}

/*
 * check-name: cond-one-void-pedantic
 * check-command: sparse -Wpedantic $file
 *
 * check-error-start
cond-one-void-pedantic.c:8:11: warning: ISO C forbids conditional expression with only one void side
cond-one-void-pedantic.c:9:11: warning: ISO C forbids conditional expression with only one void side
cond-one-void-pedantic.c:10:11: warning: ISO C forbids conditional expression with only one void side
cond-one-void-pedantic.c:11:11: warning: ISO C forbids conditional expression with only one void side
cond-one-void-pedantic.c:12:11: warning: ISO C forbids conditional expression with only one void side
cond-one-void-pedantic.c:14:18: warning: ISO C forbids conditional expression with only one void side
cond-one-void-pedantic.c:19:19: warning: ISO C forbids conditional expression with only one void side
cond-one-void-pedantic.c:19:19: warning: incorrect type in initializer (different base types)
cond-one-void-pedantic.c:19:19:    expected int x
cond-one-void-pedantic.c:19:19:    got void
 * check-error-end
 */
