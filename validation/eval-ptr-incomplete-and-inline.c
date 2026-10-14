/* 1. Subtraction of pointers to incomplete type */
extern int arr[];
static int test_sub(int (*p)[], int (*q)[])
{
	return p - q;
}

/* 2. _Alignof on incomplete type (void) */
static unsigned long test_align(void)
{
	return _Alignof(void);
}

/* 3. Float <-> unsigned integer constant folding in initializers */
static unsigned int u = (unsigned int)3.8f;
static float f = 4000000000U;

/* 4. Unsigned pointer arithmetic */
static char *test_ptr_add(char *p)
{
	return p + 0x80000000U;
}

/* 5. Named asm constraint */
static int test_asm_named(int val)
{
	int out;
	asm("mov %0, %[in]" : "=r"(out) : [in] "r"(val));
	return out;
}

/* 6. Inlining with local array and union cast */
union U { int x; float y; };
static inline int inl_func(int v)
{
	int a[4] = { v, 1, 2, 3 };
	union U u = (union U)v;
	return a[0] + u.x;
}
static int call_inl(int x, int y)
{
	return inl_func(x) + inl_func(y);
}

/* 7. Inlining multi-dimensional VLAs, VLA pointers, and typeof(VLA) */
static inline unsigned long inl_vla(int n, int m)
{
	int mat[n][m];
	int (*p)[m] = mat;
	typeof(mat) mat2;

	for (int row[n], k = 0; k < 1; k++)
		row[0] = n;
	return sizeof(mat) + sizeof(*p) + sizeof(mat2) + sizeof(int [n][m]);
}

static unsigned long call_inl_vla(int a, int b, int c, int d)
{
	return inl_vla(a, b) + inl_vla(c, d);
}

/*
 * check-name: eval-ptr-incomplete-and-inline
 * check-command: sparse -Wno-vla $file
 *
 * check-error-start
eval-ptr-incomplete-and-inline.c:5:18: error: subtraction of pointers to incomplete type
eval-ptr-incomplete-and-inline.c:11:16: error: cannot size alignment of incomplete type
 * check-error-end
 */
