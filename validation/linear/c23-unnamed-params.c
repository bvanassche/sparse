int f3(int a, double, int b);
int f3(int a, double, int b)
{
	return a + b;
}

static inline int inl(int a, int, int b)
{
	return a + b;
}

extern int side_effect(void);

int caller(void);
int caller(void)
{
	return inl(10, side_effect(), 20);
}

/*
 * check-name: linear-c23-unnamed-params
 * check-command: test-linearize -m64 -std=c23 $file
 *
 * check-output-ignore
 * check-output-contains: add\\.32 *%r[0-9]* <- %arg1, %arg3
 * check-output-contains: call\\.32 *%r[0-9]* <- side_effect
 */
