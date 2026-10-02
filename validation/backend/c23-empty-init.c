typedef typeof(nullptr) nullptr_t;

struct s {
	int a;
	void *p;
};

static nullptr_t g_np = {};
static void *g_ptr = {};
static struct s g_s = {};

int test_vla_empty(int n);
int test_vla_empty(int n)
{
	int vla[n] = {};
	return vla[0] + g_s.a + (g_np != nullptr) + (g_ptr != nullptr);
}

/*
 * check-name: backend-c23-empty-init
 * check-command: sparse-llvm-dis -m64 -std=c23 -Wno-vla $file
 * check-output-ignore
 * check-output-contains: define i32 @test_vla_empty(
 */
