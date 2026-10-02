/*
 * Verify that self-referencing constexpr initializers do not cause
 * infinite recursion or stack overflow in constant_symbol_value().
 */

static constexpr int a = a;
static constexpr int b = b + 1;

static void test_local(void)
{
	constexpr int x = x;
	constexpr int y = y + 1;
	int c = 5;
	{
		constexpr int c = c;
		(void)c;
	}
	constexpr int sz = sizeof(sz);
	(void)sz;
	(void)c;
}

static void test_eval(void)
{
	_Static_assert(b == 0, "");
}

/*
 * check-name: C23 constexpr self-initialization recursion
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-constexpr-self-init.c:7:26: error: underspecified symbol 'b' used in its own initializer
c23-constexpr-self-init.c:25:26: error: bad constant expression
c23-constexpr-self-init.c:6:26: error: underspecified symbol 'a' used in its own initializer
c23-constexpr-self-init.c:11:27: error: underspecified symbol 'x' used in its own initializer
c23-constexpr-self-init.c:12:27: error: underspecified symbol 'y' used in its own initializer
c23-constexpr-self-init.c:15:35: error: underspecified symbol 'c' used in its own initializer
c23-constexpr-self-init.c:18:35: error: underspecified symbol 'sz' used in its own initializer
 * check-error-end
 */
