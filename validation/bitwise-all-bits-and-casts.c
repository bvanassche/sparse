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

	return a | b | c | d | e | f;
}

static be64 test_all_bits_64(void)
{
	be64 a = ~0;
	be64 b = ~0ULL;
	be64 c = -1;

	return a | b | c;
}

static be16 test_narrow_all_bits(void)
{
	be16 a = ~0;
	be16 b = 0xffff;
	return a | b;
}

/*
 * check-name: bitwise-all-bits-and-casts
 * check-command: sparse -Wbitwise $file
 */
