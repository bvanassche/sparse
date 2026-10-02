static_assert(__has_c_attribute(maybe_unused) != 0);
static_assert(__has_c_attribute(nodiscard) != 0);

[[maybe_unused]] static int unused_var;

[[nodiscard]] static int get_val(void)
{
	return 42;
}

[[deprecated("use new_func instead")]] static void old_func(void)
{
}

[[noreturn]] static void fatal(void)
{
	for (;;) {
	}
}

[[_Noreturn]] static void fatal2(void)
{
	for (;;) {
	}
}

static int pure_add(int a, int b) [[unsequenced]]
{
	return a + b;
}

static int pure_mul(int a, int b) [[reproducible]]
{
	return a * b;
}

static int (*fp_unseq)(int, int) [[unsequenced]] = pure_add;
typedef int (*plain_int_fp_t)(int);
typedef int (__attribute__((__pure__)) *repro_int_fp_t)(int);
static int plain_inc(int a)
{
	return a + 1;
}
static int repro_inc(int a) [[reproducible]]
{
	return a + 1;
}
static int (*get_plain_fp(void) [[reproducible]])(int)
{
	return plain_inc;
}
static int (*get_repro_fp(void))(int) [[reproducible]]
{
	return repro_inc;
}
static_assert([typeof(&get_plain_fp)] ==
	      [plain_int_fp_t (__attribute__((__pure__)) *)(void)],
	      "inner [[]] applies to outer function");
static_assert([typeof(get_plain_fp())] == [plain_int_fp_t],
	      "inner [[]] does not leak to returned function pointer");
static_assert([typeof(&get_repro_fp)] == [repro_int_fp_t (*)(void)],
	      "outer [[]] does not leak to outer function");
static_assert([typeof(get_repro_fp())] == [repro_int_fp_t],
	      "outer [[]] applies to returned function pointer");

struct [[deprecated]] S {
	[[maybe_unused]] int field;
};

enum [[deprecated]] E {
	ITEM1 [[deprecated]],
	ITEM2,
};

typedef int T_attr_label;

static void param_fn([[maybe_unused]] int a, int b [[maybe_unused]],
		     [[maybe_unused]] int)
{
	(void)a;
	(void)b;
}

struct [[gnu::packed]] PackedStruct {
	char c;
	int i;
};
static_assert(sizeof(struct PackedStruct) == sizeof(char) + sizeof(int),
	      "packed");

struct [[gnu::aligned(16)]] AlignedStruct {
	int x;
};
static_assert(alignof(struct AlignedStruct) == 16, "align");

[[gnu::aligned(16)]] static int aligned_var;

[[maybe_unused]] [[nodiscard]] static int multi_attr(void)
{
	return 0;
}

[[maybe_unused, nodiscard]] static int comma_attr(void)
{
	return 0;
}

[[]] static int empty_attr;
[[
	,
]] static int empty_attr2;

static int test_statements(int val)
{
	[[maybe_unused]] int x = val;
	if (x < 0)
		goto lbl1;
	switch (x) {
	case 1:
		x++;
		[[fallthrough]];
	case 2:
		return x;
	[[maybe_unused]] case 3:
		return 3;
	[[maybe_unused]] default:
		break;
	}

lbl1:
	[[]];
[[maybe_unused]] lbl2:
	[[maybe_unused]] int y = 10;
	if (!y)
		goto T_attr_label;
[[maybe_unused]] T_attr_label:
	return y;
}

[[clang::annotate_type("ptr_attr", void *)]] static int *vendor_attr_var;
[[vendor::custom(struct foo, (1 + 2))]] static int vendor_attr_var2;

static void fn_after_ident [[deprecated]] (void)
{
}

static int arr_after_ident [[maybe_unused]] [4];

/*
 * check-name: C23 [[]] standard attribute syntax
 * check-command: sparse -std=c23 $file
 */
