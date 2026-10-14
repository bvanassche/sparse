#define __noderef __attribute__((noderef))
#define __nocast __attribute__((nocast))

struct empty {};
struct inc;
struct __attribute__((aligned(16))) inc_aligned;

extern struct empty es;
extern int ext_arr[];

_Static_assert(_Generic(es, struct empty: 1, default: 0) == 1, "empty struct generic");
_Static_assert(_Generic(0, int[4]: 1, default: 2) == 2, "fixed array assoc");
_Static_assert(_Alignof(struct empty) == 1, "empty struct alignof");

static unsigned long valid_alignof(int n)
{
	return _Alignof(int[n]) + _Alignof(void(void)) + __alignof__(ext_arr);
}

static void *valid_void_ptr_add(void *p)
{
	return p + 1;
}

static void test_incomplete_errors(struct inc *pi, int (*pa)[], int n)
{
	(void)(pi + 1);
	(void)(pa + 1);
	(void)_Alignof(void);
	(void)_Alignof(int[]);
	(void)_Alignof(struct inc);
	(void)_Alignof(struct inc_aligned);
	(void)_Alignof(struct inc[2]);
	(void)_Generic(0, int[2][n]: 1, default: 0);
	(void)_Generic(0, struct inc[2]: 1, default: 0);
}

static long test_qualifiers(int c, void *vp, void __nocast *vp_nc, int *p,
			    int __noderef *p_nd, int __nocast *p_nc)
{
	void __nocast *ok_both_nc = c ? vp_nc : p_nc;
	void __nocast *ok_one_nc = c ? vp : p_nc;
	void __noderef *ok_nd = c ? vp : p_nd;
	int *bad_nc = c ? vp_nc : p_nc;
	int *bad_nd = c ? vp : p_nd;
	long r = 0;

	(void)ok_both_nc;
	(void)ok_one_nc;
	(void)ok_nd;
	(void)bad_nc;
	(void)bad_nd;

	r += (p_nd == p);
	r += (p_nd - p);
	r += (p_nc == p);
	r += (p_nc - p);
	return r;
}

/*
 * check-name: eval-type-checks
 * check-command: sparse -Wno-vla $file
 *
 * check-error-start
eval-type-checks.c:27:19: error: arithmetics on pointers to incomplete type
eval-type-checks.c:28:19: error: arithmetics on pointers to incomplete type
eval-type-checks.c:29:15: error: cannot size alignment of incomplete type
eval-type-checks.c:30:15: error: cannot size alignment of incomplete type
eval-type-checks.c:31:15: error: cannot size alignment of incomplete type
eval-type-checks.c:32:15: error: cannot size alignment of incomplete type
eval-type-checks.c:33:15: error: cannot size alignment of incomplete type
eval-type-checks.c:34:27: error: variable length array type in generic selection
eval-type-checks.c:35:27: error: incomplete type in generic selection
eval-type-checks.c:42:40: warning: implicit cast to nocast type
eval-type-checks.c:44:25: warning: implicit cast from nocast type
eval-type-checks.c:45:25: warning: incorrect type in initializer (different modifiers)
eval-type-checks.c:45:25:    expected int *bad_nd
eval-type-checks.c:45:25:    got void [noderef] *
eval-type-checks.c:54:20: error: incompatible types in comparison expression (different modifiers):
eval-type-checks.c:54:20:    int [noderef] *
eval-type-checks.c:54:20:    int *
eval-type-checks.c:55:20: error: subtraction of different types can't work (different modifiers)
eval-type-checks.c:56:20: error: incompatible types in comparison expression (different modifiers):
eval-type-checks.c:56:20:    int [nocast] *
eval-type-checks.c:56:20:    int *
eval-type-checks.c:57:20: error: subtraction of different types can't work (different modifiers)
 * check-error-end
 */
