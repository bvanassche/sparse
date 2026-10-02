struct s {
	int a;
	void *ptr;
};

struct nested {
	struct s inner;
	int arr[2];
};

union u {
	int a;
	char *p;
};

enum e {
	E0,
	E1,
};

typedef typeof(nullptr) nullptr_t;

static int i = {};
static float f = {};
static double d = {};
static void *p = {};
static bool b = {};
static nullptr_t np = {};
static enum e ge = {};
static struct s s = {};
static struct s s2 = { {}, {} };
static struct nested ns = { {}, {} };
static union u u = {};
static int arr[3] = {};

static constexpr int ci = {};
static constexpr bool cb = {};
static_assert(ci == 0, "ci");
static_assert(cb == false, "cb");

static int test(int n)
{
	int local_i = {};
	void *local_p = {};
	struct s local_s = {};
	int local_arr[2] = {};
	int vla[n] = {};
	int cl_i = (int){};
	struct s cl_s = (struct s){};
	int *cl_arr = (int [3]){};

	(void)local_p;
	(void)local_s;
	(void)cl_s;
	return local_i + local_arr[0] + vla[0] + cl_i + cl_arr[0] +
	       i + (int)f + (int)d + (p != nullptr) + b + (np != nullptr) +
	       ge + s.a + s2.a + (s2.ptr != nullptr) +
	       ns.inner.a + u.a + arr[0];
}

/*
 * check-name: c23 empty initializer braces
 * check-command: sparse -std=c23 -pedantic -Wno-vla $file
 */
