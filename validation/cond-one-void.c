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
	1 ?: g();
	0 ?: g();
	c ? g() : (void)0;		// both sides void: always OK
	(void)(c ? h() : g());
}

struct empty {};
struct incomplete;
extern struct empty emp;
extern struct incomplete inc;

static int use_value(int c)
{
	int x = c ? h() : g();		// KO: void value used
	int y = 1 ?: g();		// KO: void value used
	c ? (void)0 : emp;		// OK: complete empty struct
	c ? (void)0 : inc;		// KO: incomplete type
	return x + y;
}

/*
 * check-name: cond-one-void
 * check-description: Like gcc and clang, accept conditional expressions
 *	with only one void side and give these type void.
 *
 * check-error-start
cond-one-void.c:26:19: warning: incorrect type in initializer (different base types)
cond-one-void.c:26:19:    expected int x
cond-one-void.c:26:19:    got void
cond-one-void.c:27:19: warning: incorrect type in initializer (different base types)
cond-one-void.c:27:19:    expected int y
cond-one-void.c:27:19:    got void
cond-one-void.c:29:11: error: expression has incomplete type 'struct incomplete'
 * check-error-end
 */
