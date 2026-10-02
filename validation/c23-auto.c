#ifdef __CHECKER__
#define ASSERT_TYPE(expr, T) _Static_assert([typeof(expr)] == [T], "")
#else
#define ASSERT_TYPE(expr, T) \
	_Static_assert(_Generic((typeof(expr) *)0, typeof(T) *: 1, default: 0), "")
#endif

struct s {
	int a;
};
static void fn(void)
{
}

static auto file_static = 42L;
ASSERT_TYPE(file_static, long);

static void test(void)
{
	auto i = 0;
	ASSERT_TYPE(i, int);

	auto l = 1L;
	ASSERT_TYPE(l, long);

	auto d = 1.0;
	ASSERT_TYPE(d, double);

	auto f = 1.0f;
	ASSERT_TYPE(f, float);

	auto p = &i;
	ASSERT_TYPE(p, int *);

	const auto ci = 42;
	ASSERT_TYPE(ci, const int);

	volatile auto vi = 1;
	ASSERT_TYPE(vi, volatile int);

	_Atomic auto ai = 2;
	ASSERT_TYPE(ai, _Atomic int);

	/* Top-level qualifiers and _Atomic are stripped by lvalue conversion */
	auto stripped_c = ci;
	ASSERT_TYPE(stripped_c, int);
	stripped_c = 43;

	auto stripped_v = vi;
	ASSERT_TYPE(stripped_v, int);
	stripped_v = 2;

	auto stripped_a = ai;
	ASSERT_TYPE(stripped_a, int);
	stripped_a = 3;

	/* Pointee qualifiers and decayed array element qualifiers are kept */
	const int *cp = &ci;
	auto p_to_const = cp;
	ASSERT_TYPE(p_to_const, const int *);

	int *const top_const_p = &i;
	auto stripped_p = top_const_p;
	ASSERT_TYPE(stripped_p, int *);
	stripped_p = nullptr;

	const int carr[2] = { 1, 2 };
	auto decayed_carr = carr;
	ASSERT_TYPE(decayed_carr, const int *);

	static auto si = 100;
	ASSERT_TYPE(si, int);

	auto s = "hello";
	ASSERT_TYPE(s, char *);

	int arr[10];
	auto a = arr;
	ASSERT_TYPE(a, int *);

	auto fp = fn;
	ASSERT_TYPE(fp, void (*)(void));

	struct s st = { 1 };
	auto sv = st;
	ASSERT_TYPE(sv, struct s);

	const struct s cst = { 2 };
	auto stripped_st = cst;
	ASSERT_TYPE(stripped_st, struct s);
	stripped_st.a = 3;

	auto int normal = 1;
	ASSERT_TYPE(normal, int);

	int auto normal2 = 2;
	ASSERT_TYPE(normal2, int);

	for (auto idx = 0; idx < 1; idx++) {
		ASSERT_TYPE(idx, int);
	}
}

/*
 * check-name: C23 auto type inference
 * check-command: sparse -std=c23 $file
 */
