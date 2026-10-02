#if true
int t = 1;
#endif

#if !false
int f = 1;
#endif

#if true + true != 2
#error "true + true must equal 2 in preprocessor"
#endif

bool a = true;
bool b = false;

struct bool_bf {
	bool flag : 1;
};

static_assert(true == 1, "true is 1");
static_assert(false == 0, "false is 0");
static_assert(true + true == 2, "integer promotion of true");
static_assert(sizeof(true) == sizeof(bool), "sizeof true is sizeof bool");
static_assert(sizeof(false) == sizeof(bool), "sizeof false is sizeof bool");
static_assert(_Generic(true, bool: 1, int: 0, default: 0) == 1,
	      "type of true is bool");
static_assert(_Generic(false, bool: 1, int: 0, default: 0) == 1,
	      "type of false is bool");
static_assert(_Generic(+true, int: 1, bool: 0, default: 0) == 1,
	      "+true promotes to int");

static int test_conversions(void)
{
	bool b1 = (bool)0.5;
	bool b2 = (bool)0.0;
	bool b3 = (bool)&a;
	struct bool_bf s = { .flag = true };

	return b1 + !b2 + b3 + s.flag;
}

/*
 * check-name: c23-bool
 * check-command: sparse -std=c23 -pedantic -Wno-decl $file
 */
