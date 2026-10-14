extern void fn_noreturn(void) __attribute__((noreturn));
_Static_assert(__builtin_has_attribute(fn_noreturn, noreturn), "noreturn");
_Static_assert(__builtin_has_attribute(fn_noreturn, __noreturn__), "__noreturn__");

extern int fn_pure(int) __attribute__((pure));
_Static_assert(__builtin_has_attribute(fn_pure, pure), "pure");
_Static_assert(__builtin_has_attribute(fn_pure, __pure__), "__pure__");
_Static_assert(!__builtin_has_attribute(fn_pure, const), "pure is not const");
_Static_assert(!__builtin_has_attribute(fn_pure, __const__), "pure is not __const__");

extern int fn_const(int) __attribute__((const));
_Static_assert(__builtin_has_attribute(fn_const, const), "const");
_Static_assert(__builtin_has_attribute(fn_const, __const__), "__const__");
_Static_assert(!__builtin_has_attribute(fn_const, pure), "const is not pure");
_Static_assert(!__builtin_has_attribute(fn_const, __pure__), "const is not __pure__");

/* const on object/member declarations vs expressions/types */
static const int ci = 5;
static int vi = 5;
static const int ca[3] = { 0 };
static const int *cip;
static int * const cpi = (void *)0;

_Static_assert(__builtin_has_attribute(ci, const), "const var");
_Static_assert(!__builtin_has_attribute(vi, const), "non-const var");
_Static_assert(__builtin_has_attribute(ca, const), "const array");
_Static_assert(!__builtin_has_attribute(cip, const), "ptr to const");
_Static_assert(!__builtin_has_attribute(*cip, const), "deref ptr to const");
_Static_assert(__builtin_has_attribute(cpi, const), "const ptr");
_Static_assert(!__builtin_has_attribute(const int, const), "const int typename");

struct SConst {
	const int cfield;
	int plain;
};
static struct SConst sc;
static const struct SConst csc = { 0, 0 };

_Static_assert(__builtin_has_attribute(sc.cfield, const), "const field");
_Static_assert(__builtin_has_attribute(((struct SConst *)0)->cfield, const), "arrow const field");
_Static_assert(!__builtin_has_attribute(sc.plain, const), "plain field");
_Static_assert(!__builtin_has_attribute(csc.plain, const), "const struct plain field");
_Static_assert(!__builtin_has_attribute(((const struct SConst *)0)->plain, const), "const arrow plain field");

/* aligned, _Alignas, typedef pointer non-leakage, and member non-inheritance */
typedef int aligned_int __attribute__((aligned(16)));
static aligned_int ax;
static aligned_int *ap;
static aligned_int mx, *mp;
static _Alignas(16) int a16;
static _Alignas(0) int a0;

_Static_assert(_Alignof(ax) == 16, "ax align 16");
_Static_assert(_Alignof(ap) == sizeof(void *), "ap ptr align");
_Static_assert(_Alignof(mx) == 16, "mx align 16");
_Static_assert(_Alignof(mp) == sizeof(void *), "mp ptr align");
_Static_assert(__builtin_has_attribute(aligned_int, aligned), "typedef aligned");
_Static_assert(__builtin_has_attribute(ax, aligned), "ax aligned");
_Static_assert(!__builtin_has_attribute(ap, aligned), "ap not aligned");
_Static_assert(__builtin_has_attribute(*ap, aligned), "*ap aligned");
_Static_assert(__builtin_has_attribute(mx, aligned), "mx aligned");
_Static_assert(!__builtin_has_attribute(mp, aligned), "mp not aligned");
_Static_assert(__builtin_has_attribute(*mp, aligned), "*mp aligned");
_Static_assert(__builtin_has_attribute(a16, aligned), "_Alignas(16) aligned");
_Static_assert(!__builtin_has_attribute(a0, aligned), "_Alignas(0) not aligned");

struct SAligned {
	int a;
	int b __attribute__((aligned(8)));
} __attribute__((aligned(16)));
static struct SAligned sa;
static struct SAligned sa_var __attribute__((aligned(32)));

_Static_assert(__builtin_has_attribute(struct SAligned, aligned), "struct SAligned aligned");
_Static_assert(!__builtin_has_attribute(sa.a, aligned), "sa.a not aligned");
_Static_assert(!__builtin_has_attribute(sa_var.a, aligned), "sa_var.a not aligned");
_Static_assert(__builtin_has_attribute(sa.b, aligned), "sa.b aligned");

struct SOuter {
	struct SAligned inner;
	struct SAligned arr[2];
};
static struct SOuter *so_ptr;
static struct SAligned sa_arr[2];

_Static_assert(__builtin_has_attribute(so_ptr->inner.b, aligned), "nested member aligned");
_Static_assert(!__builtin_has_attribute(so_ptr->inner.a, aligned), "nested member not aligned");
_Static_assert(__builtin_has_attribute(sa_arr[1].b, aligned), "array subscript member aligned");
_Static_assert(!__builtin_has_attribute(sa_arr[1].a, aligned), "array subscript member not aligned");
_Static_assert(__builtin_has_attribute(so_ptr->arr[1].b, aligned), "nested array member aligned");

/* typename operands */
_Static_assert(__builtin_has_attribute(int __attribute__((aligned(8))), aligned), "typename aligned");
_Static_assert(!__builtin_has_attribute(int, aligned), "typename int not aligned");
_Static_assert(__builtin_has_attribute(struct __attribute__((packed)) { int x; }, packed), "typename pre-packed");
_Static_assert(__builtin_has_attribute(struct { int x; } __attribute__((packed)), packed), "typename post-packed");
_Static_assert(!__builtin_has_attribute(struct { int x; }, packed), "typename unpacked");
extern void (*fp_noreturn)(void) __attribute__((noreturn));
_Static_assert(__builtin_has_attribute(fp_noreturn, noreturn), "fnptr decl noreturn");
extern int (*fp_after_ident [[unsequenced]])(int);
extern int (*fn_ret_fp_a(void) [[unsequenced]])(int);
extern int (*fn_ret_fp_b(void))(int) [[unsequenced]];
extern int (*fn_ret_fp_c [[unsequenced]] (void))(int);
_Static_assert(__builtin_has_attribute(fp_after_ident, unsequenced), "fnptr after ident");
_Static_assert(__builtin_has_attribute(fn_ret_fp_a, unsequenced), "nested fn unseq");
_Static_assert(!__builtin_has_attribute(fn_ret_fp_a(), unsequenced), "ret fp not unseq");
_Static_assert(!__builtin_has_attribute(fn_ret_fp_b, unsequenced), "outer fn not unseq");
_Static_assert(__builtin_has_attribute(fn_ret_fp_b(), unsequenced), "ret fp unseq");
_Static_assert(__builtin_has_attribute(fn_ret_fp_c, unsequenced), "fn after ident unseq");
_Static_assert(!__builtin_has_attribute(fn_ret_fp_c(), unsequenced), "fn after ident ret fp");

/* Redeclaration attribute inheritance */
void ext_redecl_weak(void) __attribute__((weak));
void ext_redecl_weak(void) {}
_Static_assert(__builtin_has_attribute(ext_redecl_weak, weak), "redecl weak");

int ext_redecl_const(int x) __attribute__((const));
int ext_redecl_const(int x) { return x; }
_Static_assert(__builtin_has_attribute(ext_redecl_const, const), "redecl const");
_Static_assert(!__builtin_has_attribute(ext_redecl_const, pure), "redecl const not pure");

int ext_redecl_pure(int x) __attribute__((pure));
int ext_redecl_pure(int x) { return x; }
_Static_assert(__builtin_has_attribute(ext_redecl_pure, pure), "redecl pure");
_Static_assert(!__builtin_has_attribute(ext_redecl_pure, const), "redecl pure not const");

extern int ext_redecl_aligned __attribute__((aligned(16)));
int ext_redecl_aligned;
_Static_assert(__builtin_has_attribute(ext_redecl_aligned, aligned), "redecl aligned");
_Static_assert(_Alignof(ext_redecl_aligned) == 16, "redecl align 16");

#ifdef __CHECKER__
static int __attribute__((noderef)) *ptr_noderef;
_Static_assert(__builtin_has_attribute(*ptr_noderef, noderef), "noderef");
_Static_assert(__builtin_has_attribute(*ptr_noderef, __noderef__), "__noderef__");
_Static_assert(!__builtin_has_attribute(ptr_noderef, noderef), "ptr itself not noderef");

static int __attribute__((address_space(1))) *ptr_as;
_Static_assert(__builtin_has_attribute(*ptr_as, address_space), "address_space");
_Static_assert(__builtin_has_attribute(*ptr_as, __address_space__), "__address_space__");
_Static_assert(!__builtin_has_attribute(ptr_as, address_space), "ptr itself not address_space");

typedef unsigned int __attribute__((bitwise)) __be32;
static __be32 be_var;
static __be32 *be_ptr;
_Static_assert(__builtin_has_attribute(__be32, bitwise), "__be32 bitwise");
_Static_assert(__builtin_has_attribute(__be32, __bitwise__), "__be32 __bitwise__");
_Static_assert(__builtin_has_attribute(be_var, bitwise), "be_var bitwise");
_Static_assert(__builtin_has_attribute(*be_ptr, bitwise), "*be_ptr bitwise");
_Static_assert(!__builtin_has_attribute(be_ptr, bitwise), "be_ptr not bitwise");
_Static_assert(!__builtin_has_attribute(int, bitwise), "int not bitwise");
#endif

/*
 * check-name: builtin-has-attribute-more
 * check-command: sparse -Wno-decl $file
 */
