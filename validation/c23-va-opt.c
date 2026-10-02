#define EMPTY
#define COUNT_VA(...) 0 __VA_OPT__(+1)
static_assert(COUNT_VA() == 0, "empty");
static_assert(COUNT_VA(EMPTY) == 0, "empty macro");
static_assert(COUNT_VA(x) == 1, "one");
static_assert(COUNT_VA(x, y) == 1, "two");

#define ADD_OPT(x, ...) (x __VA_OPT__(+__VA_ARGS__))
static_assert(ADD_OPT(10) == 10, "add 1");
static_assert(ADD_OPT(10, 20) == 30, "add 2");

#define STR_OPT(...) #__VA_OPT__(__VA_ARGS__)
static_assert(sizeof(STR_OPT()) == 1, "str empty");
static_assert(sizeof(STR_OPT(EMPTY)) == 1, "str empty macro");
static_assert(sizeof(STR_OPT(abc)) == 4, "str abc");

#define PASTE_OPT(p, ...) p ## __VA_OPT__(_ok)
enum { val = 10, val_ok = 20 };
static_assert(PASTE_OPT(val) == 10, "paste empty");
static_assert(PASTE_OPT(val, 1) == 20, "paste non-empty");

static int foo(int a, ...)
{
	return a;
}

#define CALL_FOO(...) foo(0 __VA_OPT__(, __VA_ARGS__))
static int test(void)
{
	return CALL_FOO() + CALL_FOO(1) + CALL_FOO(1, 2);
}

/*
 * check-name: c23-va-opt-compile
 * check-command: sparse -std=c23 -Wno-decl $file
 */
