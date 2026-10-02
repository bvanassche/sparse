typedef typeof(nullptr) nullptr_t;

static int (nullptr);

enum bad_enum {
	E_BAD = nullptr,
	E_NEXT,
};

struct bad_bf {
	unsigned int x : nullptr;
};

static int bad_arr[nullptr];

static void test_errors(int *p, int c)
{
	nullptr_t np = nullptr;
	int i2 = (int)nullptr;
	long l2 = (long)np;
	double d2 = (double)nullptr;
	nullptr_t b1 = (nullptr_t)1;
	nullptr_t b2 = (nullptr_t)(int *)0;
	bool r1 = nullptr < nullptr;
	bool r2 = np > p;
	bool r3 = nullptr == 1;
	int t1 = c ? nullptr : 1;
	nullptr_t t0 = c ? nullptr : 0;
	void *a1 = nullptr + 1;
	int u1 = *nullptr;
	int u2 = +nullptr;
	int u3 = -nullptr;
	int u4 = ~nullptr;
	np++;

	switch (np) {
	default:
		break;
	}

	switch (c) {
	case nullptr:
		break;
	}

	(void)i2;
	(void)l2;
	(void)d2;
	(void)b1;
	(void)b2;
	(void)r1;
	(void)r2;
	(void)r3;
	(void)t1;
	(void)t0;
	(void)a1;
	(void)u1;
	(void)u2;
	(void)u3;
	(void)u4;
}

/*
 * check-name: c23-nullptr-error
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
c23-nullptr-error.c:3:12: error: Trying to use reserved word 'nullptr' as identifier
c23-nullptr-error.c:6:17: error: enumerator value for 'E_BAD' is not an integer constant
c23-nullptr-error.c:11:26: error: bad integer constant expression
c23-nullptr-error.c:11:33: error: bitfield 'x' has invalid width (0)
c23-nullptr-error.c:14:20: error: size of array has non-integer type
c23-nullptr-error.c:19:19: error: conversion from 'nullptr_t' to 'int'
c23-nullptr-error.c:20:20: error: conversion from 'nullptr_t' to 'long'
c23-nullptr-error.c:21:22: error: conversion from 'nullptr_t' to 'double'
c23-nullptr-error.c:22:25: error: conversion from 'int' to 'nullptr_t'
c23-nullptr-error.c:23:25: error: conversion from 'int *' to 'nullptr_t'
c23-nullptr-error.c:24:27: error: incompatible types for operation (<):
c23-nullptr-error.c:24:27:    nullptr_t
c23-nullptr-error.c:24:27:    nullptr_t
c23-nullptr-error.c:25:22: error: incompatible types for operation (>):
c23-nullptr-error.c:25:22:    nullptr_t [usertype] np
c23-nullptr-error.c:25:22:    int *p
c23-nullptr-error.c:26:27: error: incompatible types for operation (==):
c23-nullptr-error.c:26:27:    nullptr_t
c23-nullptr-error.c:26:27:    int
c23-nullptr-error.c:27:20: error: incompatible types in conditional expression (different types):
c23-nullptr-error.c:27:20:    nullptr_t
c23-nullptr-error.c:27:20:    int
c23-nullptr-error.c:28:26: error: incompatible types in conditional expression (different types):
c23-nullptr-error.c:28:26:    nullptr_t
c23-nullptr-error.c:28:26:    int
c23-nullptr-error.c:29:28: error: incompatible types for operation (+):
c23-nullptr-error.c:29:28:    nullptr_t
c23-nullptr-error.c:29:28:    int
c23-nullptr-error.c:30:18: error: cannot dereference this type
c23-nullptr-error.c:31:18: error: incompatible type for operation (+):
c23-nullptr-error.c:31:18:    nullptr_t
c23-nullptr-error.c:32:18: error: incompatible type for operation (-):
c23-nullptr-error.c:32:18:    nullptr_t
c23-nullptr-error.c:33:18: error: incompatible type for operation (~):
c23-nullptr-error.c:33:18:    nullptr_t
c23-nullptr-error.c:34:11: error: need scalar for ++/--
c23-nullptr-error.c:36:17: error: switch quantity is not an integer
c23-nullptr-error.c:42:14: error: incompatible types for 'case' statement
 * check-error-end
 */
