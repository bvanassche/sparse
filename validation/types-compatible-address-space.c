#define __as1 __attribute__((address_space(1)))
#define __as2 __attribute__((address_space(2)))

typedef int __as1 as1_int_t;
typedef int __as1 as1_arr_t[4];
typedef int __as1 as1_mat_t[2][4];

static void test_types_compatible_as(void)
{
	int __as1 obj, *ptr;

	/* Top-level address space on scalar / pointer itself is ignored */
	_Static_assert(__builtin_types_compatible_p(int __as1, int), "scalar as1 vs int");
	_Static_assert(__builtin_types_compatible_p(as1_int_t, int), "typedef scalar as1 vs int");
	_Static_assert(__builtin_types_compatible_p(typeof(obj), int), "typeof(obj) vs int");
	_Static_assert(__builtin_types_compatible_p(typeof(*ptr), int), "typeof(*ptr) vs int");
	_Static_assert(__builtin_types_compatible_p(int * __as1, int *), "ptr own as1 vs int *");

	/* Pointee address space must match */
	_Static_assert(!__builtin_types_compatible_p(int __as1 *, int *), "pointee as1 vs default");
	_Static_assert(!__builtin_types_compatible_p(typeof(ptr), int *), "typeof(ptr) vs int *");
	_Static_assert(!__builtin_types_compatible_p(int __as1 *, int __as2 *), "pointee as1 vs as2");
	_Static_assert(__builtin_types_compatible_p(int __as1 *, int __as1 *), "pointee as1 vs as1");

	/* Array element address space must match */
	_Static_assert(!__builtin_types_compatible_p(int __as1[4], int[4]), "array elem as1 vs default");
	_Static_assert(!__builtin_types_compatible_p(as1_arr_t, int[4]), "typedef array as1 vs int[4]");
	_Static_assert(!__builtin_types_compatible_p(typeof(as1_arr_t), int[4]), "typeof(as1_arr_t) vs int[4]");
	_Static_assert(!__builtin_types_compatible_p(int __as1[4], int __as2[4]), "array elem as1 vs as2");
	_Static_assert(__builtin_types_compatible_p(int __as1[4], int __as1[4]), "array elem as1 vs as1");
	_Static_assert(!__builtin_types_compatible_p(int __as1[2][4], int[2][4]), "2d array elem as1 vs default");
	_Static_assert(!__builtin_types_compatible_p(as1_mat_t, int[2][4]), "typedef 2d array as1 vs int[2][4]");
	_Static_assert(__builtin_types_compatible_p(int __as1[2][4], int __as1[2][4]), "2d array elem as1 vs as1");
}

/*
 * check-name: types-compatible-address-space
 * check-command: sparse $file
 */
