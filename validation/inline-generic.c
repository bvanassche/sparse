extern int a, b;
inline int c(void) { return a++; }
inline int e(int d) { return 0; }
inline unsigned f(void) { return e(_Generic(b, int: c())); }
static int g(void) { return f(); }
static int h(void) { return f(); }

inline void *cl(void)
{
	return _Generic((void *)0,
			void *: &(int){ 0 },
			default: (void *)0);
}

static void test_cl(void)
{
	cl();
}

static inline int inl_gen_vla(int n, void *p)
{
	return _Generic((int (*)[n])p,
			int (*)[n]: (int)sizeof(int [n]),
			default: 0);
}

static int test_gen_vla(int x, int y, void *p)
{
	return inl_gen_vla(x, p) + inl_gen_vla(y, p);
}

/*
 * check-name: inline-generic
 * check-command: sparse -Wno-vla $file
 */
