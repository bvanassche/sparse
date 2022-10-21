#define __force __attribute__((force))

typedef unsigned long long __attribute__((bitwise)) be64;
typedef unsigned int __attribute__((bitwise)) be32;
typedef unsigned short __attribute__((bitwise)) be16;
typedef signed int __attribute__((bitwise)) sbe32;

static be32 test_all_bits(void)
{
	be32 a = ~0;
	be32 b = ~0U;
	be32 c = -1;
	be32 d = 0;
	be32 e = +0;
	be32 f = -0;
	be32 g = (be32)0;
	be32 h = (__force be32)0;
	be32 i = (be32)~0;
	be32 j = (__force be32)~0;

	return a | b | c | d | e | f | g | h | i | j;
}

static be64 test_all_bits_64(void)
{
	be64 a = ~0;
	be64 b = ~0ULL;
	be64 c = -1;
	be64 d = (be64)0;
	be64 e = (__force be64)0;
	be64 f = (be64)~0;
	be64 g = (__force be64)~0;

	return a | b | c | d | e | f | g;
}

static be16 test_cross_cast_zero(void)
{
	be16 k = (be32)0;
	be16 l = (__force be32)0;
	be16 m = (be32)~0;
	be16 n = (__force be32)~0;
	be16 o = (unsigned short)~0;

	return k | l | m | n | o;
}

static int test_ordered_compare(be32 x, be16 y)
{
	int res = 0;
	if (x > 0)
		res++;
	if (x <= 0)
		res++;
	if (0 < x)
		res++;
	if (0 >= x)
		res++;
	if (x < ~0)
		res++;
	if (y <= 0xffff)
		res++;
	return res;
}

static void test_bad_narrow_and_signed(sbe32 sx)
{
	be64 bad_u32 = ~0U;
	be32 bad_u16 = (unsigned short)~0;
	be32 bad_c16 = (be16)~0;
	be32 bad_f16 = (__force be16)~0;
	int bad_ord = (sx > 0);

	(void)bad_u32;
	(void)bad_u16;
	(void)bad_c16;
	(void)bad_f16;
	(void)bad_ord;
}

/*
 * check-name: bitwise-all-bits-and-casts
 * check-command: sparse -Wbitwise $file
 *
 * check-error-start
bitwise-all-bits-and-casts.c:68:24: warning: incorrect type in initializer (different base types)
bitwise-all-bits-and-casts.c:68:24:    expected restricted be64 [usertype] bad_u32
bitwise-all-bits-and-casts.c:68:24:    got unsigned int
bitwise-all-bits-and-casts.c:69:25: warning: incorrect type in initializer (different base types)
bitwise-all-bits-and-casts.c:69:25:    expected restricted be32 [usertype] bad_u16
bitwise-all-bits-and-casts.c:69:25:    got unsigned short
bitwise-all-bits-and-casts.c:70:25: warning: incorrect type in initializer (different base types)
bitwise-all-bits-and-casts.c:70:25:    expected restricted be32 [usertype] bad_c16
bitwise-all-bits-and-casts.c:70:25:    got restricted be16 [usertype]
bitwise-all-bits-and-casts.c:71:25: warning: incorrect type in initializer (different base types)
bitwise-all-bits-and-casts.c:71:25:    expected restricted be32 [usertype] bad_f16
bitwise-all-bits-and-casts.c:71:25:    got restricted be16 [usertype]
bitwise-all-bits-and-casts.c:72:24: warning: restricted sbe32 degrades to integer
 * check-error-end
 */
