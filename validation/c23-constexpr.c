#ifdef __CHECKER__
#define ASSERT_TYPE(expr, T) _Static_assert([typeof(expr)] == [T], "")
#else
#define ASSERT_TYPE(expr, T) \
	_Static_assert(_Generic((typeof(expr) *)0, typeof(T) *: 1, default: 0), "")
#endif

static constexpr int N = 5;
static constexpr auto A = 10;
static constexpr double D = 2.5;
static constexpr int TRUNC = (int)D;
constexpr int FILE_CE = 7;

struct inner_c {
	int a;
	int b;
};

struct outer_c {
	int x;
	struct inner_c in;
};

static constexpr struct outer_c OUTER = { 1, { 2, 42 } };
static constexpr struct outer_c OUTER_DESIG = { .in.b = 99 };
static constexpr struct outer_c OUTER_COPY = OUTER;

typedef int pair_t[2];
static constexpr pair_t CE_PAIR = { 10, 20 };

static constexpr int *NP = nullptr;
static constexpr const int *CNP = (void *)0;
static constexpr typeof(nullptr) NULLP = nullptr;
static constexpr int *NP2 = NP;

static constexpr char STR[] = "ok";
static constexpr bool B0 = 0;
static constexpr bool B1 = true;
static constexpr float EXACT_F = 16777216;

struct large_c {
	int pad[1024];
	int tail;
};

static constexpr struct large_c LARGE = { .tail = 77 };

enum my_enum { ENUM_VAL = N * 2 };

struct bitfield_test {
	int b : N;
};

static int array[N];

static void test(int val)
{
	static_assert(N == 5);
	static_assert(A == 10);
	static_assert(TRUNC == 2);
	static_assert(FILE_CE == 7);
	static_assert(OUTER.in.b == 42);
	static_assert(OUTER_DESIG.in.b == 99);
	static_assert(OUTER_COPY.in.b == 42);
	static_assert(LARGE.tail == 77);
	static_assert(sizeof(array) == 5 * sizeof(int));
	static_assert(ENUM_VAL == 10);
	static_assert(sizeof(STR) == 3);
	static_assert(!B0 && B1);
	static_assert((int)EXACT_F == 16777216);

	ASSERT_TYPE(A, const int);
	ASSERT_TYPE(NP, int *const);
	ASSERT_TYPE(CNP, const int *const);
	ASSERT_TYPE(NULLP, const typeof(nullptr));
	ASSERT_TYPE(NP2, int *const);
	ASSERT_TYPE(CE_PAIR[0], const int);

	/* Ensure typedef pair_t itself was not mutated to const */
	pair_t mut_pair = { 1, 2 };
	mut_pair[0] = 3;

	constexpr int LOCAL = 3;
	constexpr register int REG_LOCAL = 4;
	static_assert(LOCAL == 3);
	static_assert(REG_LOCAL == 4);
	switch (val) {
	case LOCAL:
	case REG_LOCAL:
		break;
	}

	const int *p = &N;
	(void)p;
}

/*
 * check-name: C23 constexpr object declarations
 * check-command: sparse -std=c23 $file
 */
