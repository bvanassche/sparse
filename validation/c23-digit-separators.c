#if 0
0' '
1'+'2
#endif

#if 1'000 + 2'000 == 3'000
int pp = 1;
#endif

_Static_assert(1'000'000 == 1000000, "dec");
_Static_assert(0xdead'beef == 0xdeadbeef, "hex");
_Static_assert(0XCAFE'BABE == 0xCAFEBABE, "hex upper");
_Static_assert(0b1010'0101 == 0xa5, "bin");
_Static_assert(0B1111'0000 == 0xf0, "bin upper");
_Static_assert(0'123'456 == 0123456, "oct");
_Static_assert(1'000ULL == 1000ULL, "ull");

static int a = 1'000'000;
static unsigned int b = 0xdead'beef;
static int c = 0b1010'0101;
static int d = 0'123'456;
static double e = 1'000.000'1;
static double f = 1'2.3'4e1'0;
static float g = 0x1'a.b'cp1'0f;

/*
 * check-name: c23-digit-separators
 * check-command: sparse -std=c23 -Wno-decl $file
 */
