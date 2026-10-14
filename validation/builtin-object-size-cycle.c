static void *p2;
static void *p1 = &p2;
static void *p2 = &p1;

static unsigned long test_static_max(void)
{
	return __builtin_object_size(*(void **)p1, 0);
}

static unsigned long test_static_min(void)
{
	return __builtin_object_size(*(void **)p1, 2);
}

/*
 * Self-referential local pointer definitions (SSA/def cycle through OP_ADD
 * and cast) that traverse expand_object_size()'s visited >= 64 cycle guard.
 */
static unsigned long test_self_add_max(void)
{
	char *p = p + 1;
	return __builtin_object_size(p, 0);
}

static unsigned long test_self_add_min(void)
{
	char *p = p + 1;
	return __builtin_object_size(p, 2);
}

static unsigned long test_self_cast_max(void)
{
	void *q = (void *)q;
	return __builtin_object_size(q, 0);
}

/*
 * check-name: builtin-object-size-cycle
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-contains: ret\\..*\\$-1
 * check-output-contains: ret\\..*\\$0
 */
