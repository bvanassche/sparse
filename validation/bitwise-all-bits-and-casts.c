typedef unsigned long long __attribute__((bitwise)) be64;
typedef unsigned int __attribute__((bitwise)) be32;
typedef unsigned short __attribute__((bitwise)) be16;

static be32 test_all_bits(void)
{
	be32 a = ~0;
	be32 b = ~0U;
	be32 c = -1;
	be32 d = 0;
	be32 e = +0;
	be32 f = -0;
	be32 g = (be32)0;
	be32 i = (be32)~0;

	return a | b | c | d | e | f | g | i;
}

static be64 test_all_bits_64(void)
{
	be64 a = ~0;
	be64 b = ~0ULL;
	be64 c = -1;
	be64 d = (be64)0;
	be64 f = (be64)~0;

	return a | b | c | d | f;
}

static be16 test_cross_cast_zero(void)
{
	be16 k = (be32)0;
	be16 m = (be32)~0;
	be16 o = (unsigned short)~0;

	return k | m | o;
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

/*
 * check-name: bitwise-all-bits-and-casts
 * check-command: sparse -Wbitwise $file
 */
