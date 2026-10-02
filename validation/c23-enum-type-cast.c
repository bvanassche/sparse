/*
 * Check enumerator types and explicit/implicit casts involving enums
 * with fixed underlying types.
 */

enum E1 : unsigned long long {
	A = 0,
	B = 1ULL << 40,
};

_Static_assert(_Generic(A, unsigned long long: 1, default: 0) == 1, "A type");
_Static_assert(_Generic(B, unsigned long long: 1, default: 0) == 1, "B type");
_Static_assert(sizeof(A) == sizeof(unsigned long long), "A size");
_Static_assert(sizeof(B) == sizeof(unsigned long long), "B size");
_Static_assert(A == 0, "A value");
_Static_assert(B == (1ULL << 40), "B value");
_Static_assert((enum E1)(1ULL << 40) == B, "cast to E1");
_Static_assert((unsigned long long)B == (1ULL << 40), "cast from B");
_Static_assert((short)A == 0, "cast A to short");
_Static_assert((unsigned short)B == 0, "narrow cast B to ushort");

enum E2 : long long {
	C = -1LL,
	D = 1LL << 40,
};

_Static_assert(_Generic(C, long long: 1, default: 0) == 1, "C type");
_Static_assert(_Generic(D, long long: 1, default: 0) == 1, "D type");
_Static_assert(sizeof(C) == sizeof(long long), "C size");
_Static_assert(sizeof(D) == sizeof(long long), "D size");
_Static_assert((enum E2)-1 == C, "cast -1 to E2");

enum U8 : unsigned char {
	U8_ZERO = 0,
};

_Static_assert((enum U8)200 == 200, "narrow u8 cast");
_Static_assert((enum U8)258 == 2, "out-of-range u8 cast wraps");
_Static_assert((enum U8)-1 == 255, "negative u8 cast wraps");
_Static_assert(_Generic((enum U8)200, enum U8: 1, int: 0, default: 0) == 1,
	       "cast yields enum U8");
_Static_assert(_Generic((enum U8)200, unsigned char: 1, int: 0, default: 0) == 1,
	       "enum U8 compatible with unsigned char");

enum S8 : signed char {
	S8_NEG = -1,
};

_Static_assert((enum S8)-128 == -128, "narrow s8 cast");
_Static_assert(_Generic((enum S8)0, signed char: 1, int: 0, default: 0) == 1,
	       "enum S8 compatible with signed char");

enum B : bool {
	B_FALSE = false,
	B_TRUE = true,
};

_Static_assert((enum B)2 == 1, "bool enum cast from 2");
_Static_assert((enum B)-5 == 1, "bool enum cast from -5");
_Static_assert((enum B)0 == 0, "bool enum cast from 0");

/*
 * check-name: C23 enum underlying type casting
 * check-command: sparse -std=c23 $file
 *
 * check-error-start
c23-enum-type-cast.c:20:32: warning: cast truncates bits from constant value (10000000000 becomes 0)
c23-enum-type-cast.c:38:25: warning: cast truncates bits from constant value (102 becomes 2)
 * check-error-end
 */
