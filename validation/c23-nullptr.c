typedef typeof(nullptr) nullptr_t;

static_assert(sizeof(nullptr) == sizeof(void *), "sizeof nullptr");
static_assert(sizeof(nullptr_t) == sizeof(void *), "sizeof nullptr_t");
static_assert(alignof(nullptr_t) == alignof(void *), "alignof nullptr_t");
static_assert(_Generic(nullptr, nullptr_t: 1, void *: 0, int: 0, default: 0) == 1,
	      "typeof nullptr is nullptr_t");

void *p = nullptr;
int *q = nullptr;
nullptr_t np = nullptr;
static nullptr_t np_zero = 0;
static nullptr_t np_void0 = (void *)0;
static void (*fp_global)(void) = nullptr;

static void test_call(int *arg1, nullptr_t arg2)
{
	(void)arg1;
	(void)arg2;
}

static void test_varargs(int n, ...)
{
	(void)n;
}

static nullptr_t test_ret_np(void)
{
	return nullptr;
}

static int *test_ret_ptr(void)
{
	return nullptr;
}

static void dummy_fn(void)
{
}

static void test_func(const int *cq_const)
{
	p = np;
	q = np;
	np = 0;
	np = (void *)0;
	np = (nullptr_t)nullptr;
	np = (const nullptr_t)np;
	np = test_ret_np();
	q = test_ret_ptr();

	void (*fp)(void) = np;
	fp = nullptr;
	if (fp == nullptr || nullptr != dummy_fn) {
	}

	test_call(nullptr, nullptr);
	test_call(np, np);
	test_call(np_zero, np_void0);
	test_varargs(2, nullptr, np);

	(void)nullptr;
	(void)np;
	(void)fp_global;

	bool b1 = nullptr;
	bool b2 = np;
	bool b3 = (bool)nullptr;
	bool b4 = (bool)np;
	(void)b1;
	(void)b2;
	(void)b3;
	(void)b4;

	if (nullptr) {
	}
	if (!nullptr && !np) {
	}
	if (p == nullptr && nullptr == q) {
	}
	if (np == p && p == np) {
	}
	if (np == nullptr && np == 0 && 0 == np && np == (void *)0) {
	}

	int cond = 1;
	void *cp = cond ? nullptr : p;
	int *cq = cond ? q : nullptr;
	const int *ccq = cond ? np : cq_const;
	void *cv0 = cond ? nullptr : (void *)0;
	nullptr_t cnp1 = cond ? np : nullptr;

	static_assert(_Generic(1 ? nullptr : (int *)0, int *: 1, default: 0) == 1);
	static_assert(_Generic(1 ? nullptr : (void *)0, void *: 1, default: 0) == 1);
	static_assert(_Generic(1 ? np : nullptr, nullptr_t: 1, default: 0) == 1);

	(void)cp;
	(void)cq;
	(void)ccq;
	(void)cv0;
	(void)cnp1;
}

/*
 * check-name: c23-nullptr
 * check-command: sparse -std=c23 -pedantic -Wno-decl -Wno-non-pointer-null $file
 */
