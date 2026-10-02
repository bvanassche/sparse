/*
 * Check C23 bit-precise integer types: _BitInt(N) and unsigned _BitInt(N)
 */

_Static_assert(__BITINT_MAXWIDTH__ >= 64, "BITINT_MAXWIDTH");

/* Basic declarations */
static _BitInt(2) s2;
static _BitInt(7) s7;
static _BitInt(8) s8;
static _BitInt(9) s9;
static _BitInt(16) s16;
static _BitInt(24) s24;
static _BitInt(32) s32;
static _BitInt(40) s40;
static _BitInt(64) s64;
static _BitInt(65) s65;
static _BitInt(128) s128;

static unsigned _BitInt(1) u1;
static unsigned _BitInt(7) u7;
static unsigned _BitInt(8) u8;
static unsigned _BitInt(9) u9;
static unsigned _BitInt(16) u16;
static unsigned _BitInt(24) u24;
static unsigned _BitInt(32) u32;
static unsigned _BitInt(40) u40;
static unsigned _BitInt(64) u64;
static unsigned _BitInt(65) u65;
static unsigned _BitInt(128) u128;

/* Sizing */
_Static_assert(sizeof(s2) == 1, "s2 size");
_Static_assert(sizeof(s7) == 1, "s7 size");
_Static_assert(sizeof(s8) == 1, "s8 size");
_Static_assert(sizeof(s9) == 2, "s9 size");
_Static_assert(sizeof(s16) == 2, "s16 size");
_Static_assert(sizeof(s24) == 4, "s24 size");
_Static_assert(sizeof(s32) == 4, "s32 size");
_Static_assert(sizeof(s40) == 8, "s40 size");
_Static_assert(sizeof(s64) == 8, "s64 size");
_Static_assert(sizeof(s65) == 16, "s65 size");
_Static_assert(sizeof(s128) == 16, "s128 size");

_Static_assert(sizeof(u1) == 1, "u1 size");
_Static_assert(sizeof(u7) == 1, "u7 size");
_Static_assert(sizeof(u8) == 1, "u8 size");
_Static_assert(sizeof(u9) == 2, "u9 size");
_Static_assert(sizeof(u16) == 2, "u16 size");
_Static_assert(sizeof(u24) == 4, "u24 size");
_Static_assert(sizeof(u32) == 4, "u32 size");
_Static_assert(sizeof(u40) == 8, "u40 size");
_Static_assert(sizeof(u64) == 8, "u64 size");
_Static_assert(sizeof(u65) == 16, "u65 size");
_Static_assert(sizeof(u128) == 16, "u128 size");

/* Alignment */
_Static_assert(_Alignof(_BitInt(7)) == 1, "align 7");
_Static_assert(_Alignof(_BitInt(12)) == 2, "align 12");
_Static_assert(_Alignof(_BitInt(24)) == 4, "align 24");
_Static_assert(_Alignof(_BitInt(40)) == 8, "align 40");
_Static_assert(_Alignof(_BitInt(65)) == 8, "align 65");

/* wb and uwb literal suffixes */
_Static_assert(_Generic(0wb, _BitInt(2): 1, default: 0) == 1, "0wb");
_Static_assert(_Generic(1wb, _BitInt(2): 1, default: 0) == 1, "1wb");
_Static_assert(_Generic(1WB, _BitInt(2): 1, default: 0) == 1, "1WB");
_Static_assert(_Generic(0uwb, unsigned _BitInt(1): 1, default: 0) == 1, "0uwb");
_Static_assert(_Generic(1uwb, unsigned _BitInt(1): 1, default: 0) == 1, "1uwb");
_Static_assert(_Generic(1UWB, unsigned _BitInt(1): 1, default: 0) == 1, "1UWB");
_Static_assert(_Generic(1wbU, unsigned _BitInt(1): 1, default: 0) == 1, "1wbU");
_Static_assert(_Generic(42wb, _BitInt(7): 1, default: 0) == 1, "42wb");
_Static_assert(_Generic(42uwb, unsigned _BitInt(6): 1, default: 0) == 1,
	       "42uwb");

/* Type compatibility and distinctness */
_Static_assert(_Generic(s8, _BitInt(8): 1, char: 0, signed char: 0, default: 0) == 1,
	       "distinct s8");
_Static_assert(_Generic(u8, unsigned _BitInt(8): 1, unsigned char: 0, default: 0) == 1,
	       "distinct u8");
_Static_assert(_Generic(s24, _BitInt(24): 1, default: 0) == 1, "distinct s24");
_Static_assert(_Generic(s32, _BitInt(32): 1, int: 0, default: 0) == 1,
	       "distinct s32");
_Static_assert(_Generic(u32,
			       unsigned _BitInt(32): 1,
			       unsigned int: 0,
			       default: 0) == 1,
	       "distinct u32");

/* Compound types and bit-fields */
struct S {
	_BitInt(7) a;
	unsigned _BitInt(9) b;
};
_Static_assert(sizeof(struct S) == 4, "struct size");

struct BF {
	_BitInt(5) sbf : 3;
	unsigned _BitInt(5) ubf : 3;
};
static struct BF bf_val;
_Static_assert(_Generic(+bf_val.sbf, _BitInt(5): 1, default: 0) == 1,
	       "signed _BitInt bitfield");
_Static_assert(_Generic(+bf_val.ubf, unsigned _BitInt(5): 1, default: 0) == 1,
	       "unsigned _BitInt bitfield");

static _BitInt(10) arr[5];
_Static_assert(sizeof(arr) == 5 * sizeof(_BitInt(10)), "array size");

/* Exact-width masking and >64-bit constant folding */
_Static_assert((unsigned _BitInt(3))7uwb + 1uwb == 0uwb, "u3 wrap");
_Static_assert(~0uwb == 1uwb, "u1 ~0");
_Static_assert((unsigned _BitInt(5))~0wb == 31uwb, "u5 ~0wb");
_Static_assert((unsigned _BitInt(33))0x1ffffffffULL + 1uwb == 0uwb, "u33 wrap");
_Static_assert((_BitInt(4))-8 == -8, "s4 min");

_Static_assert(_Generic(0x8000000000000000wb, _BitInt(65): 1, default: 0) == 1,
	       "2^63 wb width is 65");
_Static_assert(0x8000000000000000wb > 0wb, "+2^63 > 0");
_Static_assert(0x8000000000000000wb > -1wb, "+2^63 > -1");
_Static_assert((_BitInt(65))0xffffffffffffffffULL > 0wb, "2^64-1 in s65 > 0");
_Static_assert((_BitInt(65))0xffffffffffffffffULL != -1wb, "2^64-1 in s65 != -1");
_Static_assert(-0x8000000000000000wb < 0wb, "-2^63 < 0");
_Static_assert(0x8000000000000000wb + (-0x8000000000000000wb) == 0wb,
	       "+2^63 + -2^63 == 0");
_Static_assert(_Generic(0x1'00000000'00000000uwb,
			unsigned _BitInt(65): 1, default: 0) == 1,
	       "2^64 uwb width is 65");
_Static_assert(_Generic(0x1'00000000'00000000wb,
			_BitInt(66): 1, default: 0) == 1,
	       "2^64 wb width is 66");
_Static_assert(_Generic(0x7fffffffffffffff'ffffffffffffffffwb,
			_BitInt(128): 1, default: 0) == 1,
	       "2^127-1 wb width is 128");
_Static_assert(_Generic(0xffffffffffffffff'ffffffffffffffffuwb,
			unsigned _BitInt(128): 1, default: 0) == 1,
	       "2^128-1 uwb width is 128");
_Static_assert(0x1'00000000'00000000uwb > 0xffffffffffffffffuwb, "2^64 > 2^64-1");
_Static_assert((0x1'00000000'00000000uwb >> 64) == 1uwb, "2^64 >> 64 == 1");
_Static_assert(((unsigned _BitInt(128))1 << 100) >> 100 == 1, "1 << 100 >> 100");
_Static_assert(~(unsigned _BitInt(100))0 ==
	       ((unsigned _BitInt(128))1 << 100) - 1, "~u100");

/* Arithmetic conversions and lack of integer promotion (C23 6.3.1.1, 6.3.1.8) */
static void test_ops(void)
{
	_BitInt(40) a = 1;
	_BitInt(50) b = 2;
	_Static_assert(_Generic(a + b, _BitInt(50): 1, default: 0) == 1,
		       "a + b");

	unsigned _BitInt(40) u = 1;
	_Static_assert(_Generic(u + a, unsigned _BitInt(40): 1, default: 0) ==
			       1,
		       "u + a");
	_Static_assert(_Generic(u + b, _BitInt(50): 1, default: 0) == 1,
		       "u + b");

	_BitInt(10) x = 1;
	_BitInt(10) y = 2;
	_Static_assert(_Generic(x + y, _BitInt(10): 1, default: 0) == 1,
		       "narrow _BitInt does not promote to int");
	_Static_assert(_Generic(+x, _BitInt(10): 1, default: 0) == 1,
		       "unary + does not promote _BitInt");

	_BitInt(32) z = 1;
	_Static_assert(_Generic(z + z, _BitInt(32): 1, default: 0) == 1,
		       "no promotion");

	int si = 1;
	unsigned int ui = 1;
	unsigned _BitInt(32) uz = 1;
	unsigned _BitInt(31) u31 = 1;
	_BitInt(33) s33 = 1;
	unsigned _BitInt(8) ub8 = 1;
	_BitInt(16) sb16 = 1;
	unsigned _BitInt(16) ub16 = 1;

	_Static_assert(_Generic(z + si, int: 1, default: 0) == 1,
		       "_BitInt(32) + int -> int");
	_Static_assert(_Generic(uz + ui, unsigned int: 1, default: 0) == 1,
		       "u_BitInt(32) + uint -> uint");
	_Static_assert(_Generic(z + ui, unsigned int: 1, default: 0) == 1,
		       "_BitInt(32) + uint -> uint");
	_Static_assert(_Generic(uz + si, unsigned int: 1, default: 0) == 1,
		       "u_BitInt(32) + int -> uint");
	_Static_assert(_Generic(u31 + si, int: 1, default: 0) == 1,
		       "u_BitInt(31) + int -> int");
	_Static_assert(_Generic(s33 + si, _BitInt(33): 1, default: 0) == 1,
		       "_BitInt(33) + int -> _BitInt(33)");
	_Static_assert(_Generic(ub8 + sb16, _BitInt(16): 1, default: 0) == 1,
		       "u_BitInt(8) + _BitInt(16) -> _BitInt(16)");
	_Static_assert(_Generic(ub16 + sb16, unsigned _BitInt(16): 1, default: 0) == 1,
		       "u_BitInt(16) + _BitInt(16) -> u_BitInt(16)");

	constexpr _BitInt(8) ce_bi8 = -128wb;
	constexpr _BitInt(70) ce_bi70 = 0x8000000000000000wb;
	_Static_assert(ce_bi8 == -128, "constexpr _BitInt(8)");
	_Static_assert(ce_bi70 > 0, "constexpr _BitInt(70) >64-bit positive");
}

struct bitint128_bf {
	unsigned _BitInt(128) f : 100;
};

static unsigned _BitInt(65) get_bitint65(void)
{
	return 1uwb;
}

/*
 * check-name: C23 _BitInt(N)
 * check-command: sparse -std=c23 $file
 */
