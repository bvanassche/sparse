enum E_char : char {
	A = 0,
	B,
};
static_assert(sizeof(enum E_char) == sizeof(char), "sizeof E_char");
static_assert(sizeof(A) == sizeof(char), "sizeof A");
static_assert(A == 0, "val A");
static_assert(B == 1, "val B");

enum E_short : short;
static_assert(sizeof(enum E_short) == sizeof(short), "sizeof E_short fwd");
enum E_short : short {
	C = 10,
	D,
};
static_assert(sizeof(enum E_short) == sizeof(short), "sizeof E_short def");
static_assert(sizeof(C) == sizeof(short), "sizeof C");
static_assert(C == 10, "val C");
static_assert(D == 11, "val D");

enum : unsigned long long {
	ULL1 = 0x100000000ULL,
	ULL2,
};
static_assert(sizeof(ULL1) == sizeof(unsigned long long), "sizeof ULL1");
static_assert(ULL1 == 0x100000000ULL, "val ULL1");
static_assert(ULL2 == 0x100000001ULL, "val ULL2");

enum E_redecl : int;
enum E_redecl : signed int;
enum E_redecl : int {
	R1 = 100,
	R2 = 200,
};
static_assert(sizeof(enum E_redecl) == sizeof(int), "sizeof E_redecl");
static_assert(R1 == 100, "val R1");
static_assert(R2 == 200, "val R2");

enum E_compat : signed short;
enum E_compat : short {
	EC1 = -10,
};
static_assert(sizeof(enum E_compat) == sizeof(short), "sizeof E_compat");
static_assert(EC1 == -10, "val EC1");

enum E_signed : signed char {
	NEG = -5,
	NEG_PLUS_ONE,
};
static_assert(sizeof(enum E_signed) == 1, "sizeof E_signed");
static_assert(NEG == -5, "val NEG");
static_assert(NEG_PLUS_ONE == -4, "val NEG_PLUS_ONE");

enum E_uchar : unsigned char {
	MAX_BYTE = 255,
};
static_assert(sizeof(enum E_uchar) == 1, "sizeof E_uchar");
static_assert(MAX_BYTE == 255, "val MAX_BYTE");

typedef unsigned short uint16_t;
enum E_typedef : uint16_t {
	T1 = 65535,
};
static_assert(sizeof(enum E_typedef) == sizeof(uint16_t), "sizeof E_typedef");
static_assert(T1 == 65535, "val T1");

enum E_bool : bool {
	EB_FALSE,
	EB_TRUE,
};
static_assert(sizeof(enum E_bool) == sizeof(bool), "sizeof E_bool");
static_assert(EB_FALSE == 0 && EB_TRUE == 1, "val E_bool");

enum E_qual : const unsigned int {
	EQ1 = 42,
};
static_assert(_Generic(EQ1, unsigned int: 1, default: 0) == 1, "EQ1 type");

enum E_brace : unsigned long long {
	EB_A = 1,
	EB_B = _Generic(EB_A, unsigned long long: 2, default: 0),
};
static_assert(EB_B == 2, "inside-brace fixed type");
static_assert(_Generic(EB_A, unsigned long long: 1, default: 0) == 1,
	      "after-brace fixed type");

struct E_unnamed_bf {
	enum E_uchar : 8;
	int x;
};
static_assert(sizeof(struct E_unnamed_bf) >= sizeof(int), "unnamed enum bitfield");

enum E_unfixed_fit {
	UF_A = 1U,
};
static_assert(_Generic(UF_A, int: 1, default: 0) == 1, "unfixed fits int");

enum E_unfixed_wide {
	UW_A = 1,
	UW_B = ~0U,
};
static_assert(_Generic(UW_A, unsigned int: 1, default: 0) == 1,
	      "C23 unfixed wide enum gives all members underlying type");
static_assert(_Generic(UW_B, unsigned int: 1, default: 0) == 1,
	      "C23 unfixed wide enum second member");

enum FwdFixed : int;
static int add_fwd_fixed_enum(enum FwdFixed x)
{
	return x + 1;
}

/*
 * check-name: C23 enumerations with fixed underlying types
 * check-command: sparse -std=c23 $file
 */
