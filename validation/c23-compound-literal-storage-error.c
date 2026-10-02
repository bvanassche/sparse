void *e = (extern int[]){ 1 };
void *t = (typedef int[]){ 1 };
int f_reg = sizeof((register int){ 1 });

static void bad_block_literals(int non_const)
{
	int *a1 = (auto int[]){ 1, 2 };
	int a2 = (auto){ 1 };
	int *tl = (thread_local int[]){ 14 };
	int *nc = (static int[]){ non_const };
	int *dup_s = (static static int[]){ 1 };
	int s_reg = (static register int){ 1 };

	(void)a1;
	(void)a2;
	(void)tl;
	(void)nc;
	(void)dup_s;
	(void)s_reg;
}

/*
 * check-name: c23-compound-literal-storage-error
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
c23-compound-literal-storage-error.c:1:12: error: 'extern' is not allowed in compound literal
c23-compound-literal-storage-error.c:2:12: error: 'typedef' is not allowed in compound literal
c23-compound-literal-storage-error.c:3:21: error: file-scope compound literal cannot specify 'register'
c23-compound-literal-storage-error.c:7:20: error: 'auto' is not allowed in compound literal
c23-compound-literal-storage-error.c:8:19: error: 'auto' is not allowed in compound literal
c23-compound-literal-storage-error.c:9:20: error: block-scope compound literal with 'thread_local' requires 'static'
c23-compound-literal-storage-error.c:11:37: error: duplicate static
c23-compound-literal-storage-error.c:12:38: error: multiple storage classes
c23-compound-literal-storage-error.c:10:35: error: non-constant initializer for static object
 * check-error-end
 */
