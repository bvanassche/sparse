/* 1. Negation of LLONG_MIN */
static long long test_neg_llong_min(void)
{
	return -(-9223372036854775807LL - 1LL);
}

/* 2. Signed bitfield post-increment assigned to another bitfield */
struct sbf {
	signed int a:3;
	signed int b:3;
	signed int c:3;
};

int bf_postinc_const(void)
{
	struct sbf s = { -1, 0, 0 };
	s.b = s.a++;
	return s.c;
}

/* 3. ((x & mask) | bits) == C with bits outside mask */
int and_or_equ(unsigned x)
{
	return ((x & 0xf0) | 0x01) == 0x11;
}

/* 4. Commuted (x + 1U) - x */
unsigned add_sub_cancel(unsigned x)
{
	return (x + 1U) - x;
}

/* 5. 0 / non-constant */
int zero_div(int x)
{
	return 0 / (x ? 1 : 2);
}

/* 6. Computed goto OP_SEL with identical targets and live PHI */
int cgoto_same_target_phi(int c, int d)
{
	void *p = c ? &&L1 : &&L1;
	int r = 1;
	if (d) {
		r = 2;
		goto L2;
	}
	goto *p;
L2:
	r = 3;
L1:
	return r;
}

/* 7. Computed goto OP_LABEL deduplication with live PHI */
int cgoto_label_dedup(int c)
{
	void *p = &&L1;
	int r = 1;
	if (c) {
		r = 2;
		goto L2;
	}
	goto *p;
L2:
	r = 3;
L1:
	return r;
}

/* 8. 128-bit zero-extension and addition above 64-bit width */
__int128 zext_u64_to_s128(void)
{
	return (__int128)0x8000000000000000ULL;
}

unsigned __int128 add_u128_carry(void)
{
	return (unsigned __int128)0x8000000000000000ULL + 0x8000000000000000ULL;
}

/* 9. ZEXT of SEXT with distinct source widths */
unsigned long long zext_sext_char(signed char c)
{
	return (unsigned long long)(unsigned int)(int)c;
}

/*
 * check-name: simplify-memops-ssa-fixes
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-excludes: VOID
 * check-output-contains: ret\\.64.*\\$0x8000000000000000
 * check-output-contains: zext\\.128
 * check-output-contains: add\\.128
 * check-output-contains: sext\\.32
 * check-output-contains: zext\\.64
 *
 * check-error-start
optim/simplify-memops-ssa-fixes.c:4:16: warning: constant integer operation overflow
 * check-error-end
 */
