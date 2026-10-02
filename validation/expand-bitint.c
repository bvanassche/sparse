/*
 * Expand constant folding tests for C23 _BitInt(N) widths != ABI bit_size
 * and widths > 64 bits.
 */

_Static_assert((~(unsigned _BitInt(37))0) == 0x1fffffffffULL, "~u37");
_Static_assert(((unsigned _BitInt(37))0x1fffffffffULL +
		(unsigned _BitInt(37))1) == 0, "wrap u37");
_Static_assert((~(_BitInt(37))0) == -1, "~s37");
_Static_assert(-(_BitInt(128))0x8000000000000000ULL < 0, "-2^63 < 0");

int cmp_u128_max(void)
{
	unsigned _BitInt(128) hi = (unsigned _BitInt(128))-1;
	unsigned _BitInt(128) lo = (unsigned _BitInt(128))0xffffffffffffffffULL;

	return hi == lo;
}

int neg_s128_bit63(void)
{
	_BitInt(128) x = (_BitInt(128))0x8000000000000000ULL;
	_BitInt(128) y = -x;

	return y < 0;
}

_BitInt(128) neg_s128_var(_BitInt(128) x)
{
	return -x;
}

double cast_u128_max_to_double(void)
{
	unsigned _BitInt(128) hi = (unsigned _BitInt(128))-1;

	return (double)hi;
}

/*
 * check-name: expand-bitint
 * check-command: test-linearize -Wno-decl -std=c23 $file
 *
 * check-output-ignore
 * check-output-contains: seteq\\.
 * check-output-contains: ret\\.32\\s*\\$1
 * check-output-contains: neg\\.128
 * check-output-contains: ucvtf\\.64
 */
