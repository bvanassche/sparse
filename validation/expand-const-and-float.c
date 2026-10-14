static const volatile int cv = 42;

int read_cv(void)
{
	return cv;
}

void vol_deref(const volatile int *p)
{
	*p;
}

static const int arr[2] = { 10, 20 };

int read_neg_off(void)
{
	return *(&arr[0] - 1);
}

unsigned int float_neg_half(void)
{
	return (unsigned int)-0.5;
}

_Bool float_half_to_bool(void)
{
	return (_Bool)0.5;
}

int float_oob_pos(void)
{
	return (int)1e30;
}

unsigned int float_neg_one(void)
{
	return (unsigned int)-1.0;
}

unsigned long long float_oob_ull(void)
{
	return (unsigned long long)1e40;
}

int float_nan(void)
{
	return (int)__builtin_nan("");
}

int float_inf(void)
{
	return (int)__builtin_inf();
}

int nonconst_float_cast(int x)
{
	return (int)(x + 0.0);
}

/*
 * check-name: expand-const-and-float
 * check-command: test-linearize -Wno-decl $file
 *
 * check-output-ignore
 * check-output-contains: load\\.32.*\\[cv\\]
 * check-output-contains: load\\.32.*%arg1
 * check-output-contains: ret\\.32.*\\$0
 * check-output-contains: ret\\.1.*\\$1
 *
 * check-error-start
expand-const-and-float.c:17:26: warning: invalid access below 'arr' (-4 8)
 * check-error-end
 */
