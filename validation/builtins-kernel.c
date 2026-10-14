#if !__has_builtin(__builtin_dynamic_object_size)
#error "missing __builtin_dynamic_object_size"
#endif

#if !__has_builtin(__builtin_has_attribute)
#error "missing __builtin_has_attribute"
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

extern int weak_var __attribute__((__weak__));
_Static_assert(__builtin_has_attribute(weak_var, __weak__), "weak_var has __weak__");
_Static_assert(__builtin_has_attribute(weak_var, weak), "weak_var has weak");
_Static_assert(__builtin_has_attribute(*&weak_var, __weak__), "*&weak_var has __weak__");

struct s_packed {
	int a;
	char b;
} __attribute__((__packed__));

_Static_assert(__builtin_has_attribute(struct s_packed, packed), "s_packed has packed");
_Static_assert(__builtin_has_attribute(struct s_packed, __packed__), "s_packed has __packed__");

struct s_normal {
	int a;
	char b;
	int c __attribute__((__packed__));
};
static struct s_normal s_norm_inst;

_Static_assert(!__builtin_has_attribute(struct s_normal, packed), "s_normal not packed");
_Static_assert(__builtin_has_attribute(s_norm_inst.c, packed), "s_norm_inst.c packed");
_Static_assert(!__builtin_has_attribute(weak_var, packed), "weak_var not packed");

static int aligned_var __attribute__((aligned(16)));
_Static_assert(__builtin_has_attribute(aligned_var, aligned), "aligned_var has aligned");

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
