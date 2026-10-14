/* 1. _Generic array/function decay and qualifier stripping */
extern int arr[4];
extern int fn(void);
const int ci = 0;

_Static_assert(_Generic(arr, int *: 1, default: 0) == 1, "array decay");
_Static_assert(_Generic(fn, int (*)(void): 1, default: 0) == 1, "func decay");
_Static_assert(_Generic(ci, int: 1, const int: 0) == 1, "qual strip");

/* 2. Inlining function with cleanup attribute and _Static_assert */
void cl(int *p);

static inline int inl_cleanup(int v)
{
	_Static_assert(sizeof(v) == sizeof(int), "int size");
	int x __attribute__((cleanup(cl))) = v;
	return x + 1;
}

int call_inl(int v)
{
	return inl_cleanup(v);
}

/* 3. Conditional operator preserving noderef qualifier */
#define __noderef __attribute__((noderef))
void cond_noderef(void *vp, int __noderef *p)
{
	int *q = (1 ? vp : p);
	(void)q;
}

/* 4. Signed division overflow in constant expression */
int div_ovf(void)
{
	return (-9223372036854775807LL - 1LL) / -1LL;
}

/*
 * check-name: eval-expand-inline-fixes
 * check-command: sparse -Wno-decl $file
 *
 * check-error-start
eval-expand-inline-fixes.c:29:21: warning: incorrect type in initializer (different modifiers)
eval-expand-inline-fixes.c:29:21:    expected int *q
eval-expand-inline-fixes.c:29:21:    got void [noderef] *
eval-expand-inline-fixes.c:36:47: warning: constant integer operation overflow
 * check-error-end
 */
