/* 1. designated_init propagation on forward and post-brace declarations */
struct __attribute__((designated_init)) s_fwd;
struct s_fwd {
	int a;
	int b;
};
static struct s_fwd fwd_pos = { 1, 2 };
static struct s_fwd fwd_des = { .a = 1, .b = 2 };

struct s_post {
	int a;
	int b;
} __attribute__((designated_init));
static struct s_post post_pos = { 3, 4 };
static struct s_post post_des = { .a = 3, .b = 4 };

/* 2. transparent_union propagation on forward and post-brace declarations */
union __attribute__((__transparent_union__)) u_fwd;
union u_fwd {
	int *ip;
	long *lp;
};
void take_u_fwd(union u_fwd u);

union u_post {
	int *ip;
	long *lp;
} __attribute__((__transparent_union__));
void take_u_post(union u_post u);

void call_transparent(int *ip, long *lp)
{
	take_u_fwd(ip);
	take_u_fwd(lp);
	take_u_post(ip);
	take_u_post(lp);
}

/* 3. Function context attribute transferred through pointer return type */
int *acquire_ptr(void) __attribute__((context(0, 1)));

void good_context(void)
{
	(void)acquire_ptr();
	__context__(-1);
}

void bad_context(void)
{
	(void)acquire_ptr();
}

/* 4. typeof on restrict-qualified pointer */
void typeof_restrict(int * restrict p)
{
	typeof(p) q = p;
	(void)q;
}

/* 5. Sub-int enum auto-increment promotes to int before long/ulong */
enum {
	E_SCHAR = (signed char)127,
	E_SCHAR_NEXT,
	E_SHRT = (short)32767,
	E_SHRT_NEXT,
	E_UCHAR = (unsigned char)255,
	E_UCHAR_NEXT,
};
_Static_assert(E_SCHAR_NEXT == 128, "schar + 1");
_Static_assert(__builtin_types_compatible_p(typeof(E_SCHAR_NEXT), int), "schar + 1 is int");
_Static_assert(E_SHRT_NEXT == 32768, "short + 1");
_Static_assert(__builtin_types_compatible_p(typeof(E_SHRT_NEXT), int), "short + 1 is int");
_Static_assert(E_UCHAR_NEXT == 256, "uchar + 1");
_Static_assert(__builtin_types_compatible_p(typeof(E_UCHAR_NEXT), int), "uchar + 1 is int");

/* 6. Out-of-range address_space and aligned warnings */
int __attribute__((address_space(0x100000000ULL))) *bad_as;
int __attribute__((aligned(0x100000000ULL))) bad_align;

/*
 * check-name: parser-type-fixes
 * check-command: sparse -Wno-decl $file
 *
 * check-error-start
parser-type-fixes.c:77:34: warning: invalid address space name
parser-type-fixes.c:78:43: warning: I don't like non-power-of-2 alignments
parser-type-fixes.c:7:33: warning: in initializer for fwd_pos: positional init of field in struct s_fwd, declared with attribute designated_init
parser-type-fixes.c:7:36: warning: in initializer for fwd_pos: positional init of field in struct s_fwd, declared with attribute designated_init
parser-type-fixes.c:14:35: warning: in initializer for post_pos: positional init of field in struct s_post, declared with attribute designated_init
parser-type-fixes.c:14:38: warning: in initializer for post_pos: positional init of field in struct s_post, declared with attribute designated_init
parser-type-fixes.c:48:6: warning: context imbalance in 'bad_context' - wrong count at exit
 * check-error-end
 */
