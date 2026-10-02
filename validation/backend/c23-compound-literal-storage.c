static const int *g_arr = (static int[]){ 1, 2, 3 };

int *test_cl_static_scalar_addr(void);
int *test_cl_static_scalar_addr(void)
{
	return &(static int){ 42 };
}

int test_cl_llvm(void);
int test_cl_llvm(void)
{
	int *lp = (static int[]){ 7, 8, 9 };
	int *tp = (static thread_local int[]){ 14 };
	register int rv = (register int){ 12 };
	int ce = (constexpr int){ 42 };

	lp[0] += 1;
	return g_arr[0] + lp[0] + tp[0] + rv + ce;
}

/*
 * check-name: backend-c23-compound-literal-storage
 * check-command: sparse-llvm-dis -m64 -std=c23 $file
 * check-output-ignore
 * check-output-contains: private global i32 42
 * check-output-contains: define i32 @test_cl_llvm(
 */
