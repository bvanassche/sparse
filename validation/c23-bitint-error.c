static _BitInt(1) a;
static unsigned _BitInt(0) b;
static _BitInt(200) c;
static _BitInt(8) bad_str[] = "abc";
static struct bad_bf {
	_BitInt(5) x : 7;
} bf_instance;
static _BitInt(8) bad_wb1 = 1wB;
static _BitInt(8) bad_wb2 = 1Wb;
enum E_bitint : _BitInt(8) { BI = 0 };
static constexpr _BitInt(4) bad_ce_bi = 8;

static void ptr_errors(_BitInt(8) *p8, _BitInt(5) *p5, char *pc,
		       signed char *psc, _BitInt(7) *p7)
{
	p8 = pc;
	p8 = psc;
	p5 = p7;
}

static int _BitInt(5) d;
static _BitInt(5) short e;
static _BitInt(128) bad_wb_ovf = 0x80000000000000000000000000000000wb;
static unsigned _BitInt(128) bad_uwb_ovf = 0x100000000000000000000000000000000uwb;

/*
 * check-name: C23 _BitInt errors
 * check-command: sparse -std=c23 -Wsparse-error $file
 * check-exit-value: 1
 *
 * check-error-start
c23-bitint-error.c:1:15: error: signed _BitInt width must be at least 2
c23-bitint-error.c:2:24: error: invalid bit width 0 for _BitInt
c23-bitint-error.c:3:15: error: _BitInt width 200 exceeds __BITINT_MAXWIDTH__ (128)
c23-bitint-error.c:8:29: error: constant 1wB is not a valid number
c23-bitint-error.c:9:29: error: constant 1Wb is not a valid number
c23-bitint-error.c:10:28: error: invalid 'enum' underlying type
c23-bitint-error.c:21:12: error: two or more data types in declaration specifiers
c23-bitint-error.c:21:20: error: Expected ) in function declarator
c23-bitint-error.c:21:20: error: got 5
c23-bitint-error.c:21:12: error: Trying to use reserved word '_BitInt' as identifier
c23-bitint-error.c:22:19: error: two or more data types in declaration specifiers
c23-bitint-error.c:22:19: error: Trying to use reserved word 'short' as identifier
c23-bitint-error.c:22:25: error: Expected ; at end of declaration
c23-bitint-error.c:22:25: error: got e
c23-bitint-error.c:23:34: error: constant 0x80000000000000000000000000000000wb is too big for _BitInt(128)
c23-bitint-error.c:24:44: error: constant 0x100000000000000000000000000000000uwb is too big for unsigned _BitInt(128)
c23-bitint-error.c:4:31: error: invalid initializer
c23-bitint-error.c:6:22: error: bitfield 'x' is wider (7) than its type (_BitInt(5))
c23-bitint-error.c:11:41: error: constexpr initializer not representable in target type
c23-bitint-error.c:16:12: error: incorrect type in assignment (different base types)
c23-bitint-error.c:16:12:    expected _BitInt(8)*p8
c23-bitint-error.c:16:12:    got char *pc
c23-bitint-error.c:17:12: error: incorrect type in assignment (different base types)
c23-bitint-error.c:17:12:    expected _BitInt(8)*p8
c23-bitint-error.c:17:12:    got signed char *psc
c23-bitint-error.c:18:12: error: incorrect type in assignment (different bit widths)
c23-bitint-error.c:18:12:    expected _BitInt(5)*p5
c23-bitint-error.c:18:12:    got _BitInt(7)*p7
 * check-error-end
 */
