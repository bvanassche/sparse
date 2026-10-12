/* Test typeof_unqual on type names */
typedef const int const_int;
typedef typeof_unqual(const_int) unqual_int;

typedef volatile int vol_int;
typedef typeof_unqual(vol_int) nonvol_int;

typedef int * const const_ptr;
typedef typeof_unqual(const_ptr) mod_ptr;

typedef _Atomic int atomic_int;
typedef typeof_unqual(atomic_int) nonatomic_int;

typedef const int ca[3];
typedef typeof_unqual(ca) unqual_ca;

typedef const int cmatrix[2][3];
typedef typeof_unqual(cmatrix) unqual_matrix;

typedef const _Atomic int const_atomic_arr[2];
typedef typeof_unqual(const_atomic_arr) unqual_atomic_arr;

typedef const int *cip_arr_t[2];
typedef int * const cpi_arr_t[2];

#ifdef __CHECKER__
typedef int __attribute__((address_space(1))) as1_int;
typedef typeof_unqual(as1_int) as0_int;
typedef int __attribute__((address_space(1))) as1_arr[2];
typedef typeof_unqual(as1_arr) as0_arr;
#endif

_Static_assert(!__builtin_types_compatible_p(ca *, int (*)[3]), "ca stays const");
_Static_assert(__builtin_types_compatible_p(ca *, const int (*)[3]), "ca is const int[3]");
_Static_assert(__builtin_types_compatible_p(unqual_ca *, int (*)[3]), "unqual_ca strips const");
_Static_assert(!__builtin_types_compatible_p(cmatrix *, int (*)[2][3]), "cmatrix stays const");
_Static_assert(__builtin_types_compatible_p(unqual_matrix *, int (*)[2][3]), "unqual_matrix strips const");
_Static_assert(!__builtin_types_compatible_p(const_atomic_arr *, _Atomic int (*)[2]), "const_atomic_arr stays const");
_Static_assert(__builtin_types_compatible_p(unqual_atomic_arr *, _Atomic int (*)[2]), "unqual_atomic_arr strips const, keeps _Atomic");
_Static_assert(__builtin_types_compatible_p(typeof_unqual(cip_arr_t) *, const int *(*)[2]), "pointee const kept");
_Static_assert(!__builtin_types_compatible_p(typeof_unqual(cip_arr_t) *, int *(*)[2]), "pointee const not stripped");
_Static_assert(__builtin_types_compatible_p(typeof_unqual(cpi_arr_t) *, int *(*)[2]), "element ptr const stripped");

static void test_types(void)
{
	unqual_int a = 1;
	a = 2;

	mod_ptr p = (void *)0;
	p = (int *)0;

	unqual_ca b;
	b[0] = 1;

	unqual_matrix u;
	u[0][0] = 1;

	unqual_atomic_arr na;
	na[0] = 1;

	typeof_unqual(cpi_arr_t) ucpi;
	ucpi[0] = (int *)0;

#ifdef __CHECKER__
	as0_int x = 0;
	int *px = &x;
	(void)px;

	as0_arr ax;
	int *pax = ax;
	(void)pax;
#endif
}

/* Test typeof_unqual on expressions */
static void test_exprs(const int c, _Atomic int a)
{
	typeof_unqual(c) uc = 10;
	uc = 20;

	typeof_unqual(a) ua = 30;
	ua = 40;

	const int arr[3] = { 0 };
	typeof_unqual(arr) uarr;
	uarr[0] = 1;
	_Static_assert(!__builtin_types_compatible_p(typeof(&arr), int (*)[3]), "arr stays const");
	_Static_assert(__builtin_types_compatible_p(typeof(&uarr), int (*)[3]), "uarr is int[3]");

	const int m[2][3] = { { 0 } };
	typeof_unqual(m) um;
	um[0][0] = 1;
	_Static_assert(!__builtin_types_compatible_p(typeof(&m), int (*)[2][3]), "m stays const");
	_Static_assert(__builtin_types_compatible_p(typeof(&um), int (*)[2][3]), "um is int[2][3]");

	const _Atomic int aa[2] = { 0 };
	typeof_unqual(aa) uaa;
	uaa[0] = 1;
	_Static_assert(!__builtin_types_compatible_p(typeof(&aa), _Atomic int (*)[2]), "aa stays const");
	_Static_assert(__builtin_types_compatible_p(typeof(&uaa), _Atomic int (*)[2]), "uaa is _Atomic int[2]");

#ifdef __CHECKER__
	static int __attribute__((address_space(1))) asa[2];
	typeof_unqual(asa) uasa;
	int *p = uasa;
	(void)p;
#endif
}

/*
 * check-name: c23-typeof-unqual
 * check-command: sparse $file
 */
