typedef typeof(nullptr) nullptr_t;

extern int sink(const char *fmt, ...);

static nullptr_t g_np = nullptr;

int test_nullptr_llvm(nullptr_t arg, int **out);
int test_nullptr_llvm(nullptr_t arg, int **out)
{
	*out = arg;
	g_np = nullptr;
	return sink("test", nullptr, arg, g_np);
}

/*
 * check-name: backend-c23-nullptr
 * check-command: sparse-llvm-dis -m64 -std=c23 $file
 * check-output-ignore
 * check-output-contains: define i32 @test_nullptr_llvm(
 * check-output-contains: @sink(
 */
