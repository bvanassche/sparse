#if !__has_builtin(__builtin_dynamic_object_size)
#error "missing __builtin_dynamic_object_size"
#endif

#if !__has_builtin(__builtin_thread_pointer)
#error "missing __builtin_thread_pointer"
#endif

#if !__has_builtin(__builtin_alloca_uninitialized)
#error "missing __builtin_alloca_uninitialized"
#endif

#if !__has_builtin(__builtin_counted_by_ref)
#error "missing __builtin_counted_by_ref"
#endif

struct fam {
	int count;
	char arr[];
};

static void test_builtins(struct fam *f)
{
	char buf[10];
	void *tp = __builtin_thread_pointer();
	void *al = __builtin_alloca_uninitialized(32);
	void *cbr = __builtin_counted_by_ref(f->arr);
	int is_null = (__builtin_counted_by_ref(f->arr) == (void *)0);
	unsigned long sz0 = __builtin_dynamic_object_size(buf, 0);
	unsigned long sz1 = __builtin_dynamic_object_size(buf, 1);

	(void)tp;
	(void)al;
	(void)cbr;
	(void)is_null;
	(void)sz0;
	(void)sz1;
}

/*
 * check-name: builtins-kernel
 */
