typedef typeof(nullptr) nullptr_t;

void test(int *p, void *v)
{
	nullptr_t np;
	np = p;
	np = v;
	np = 0;

	int i = nullptr;
	double d = np;
	(void)i;
	(void)d;
}

/*
 * check-name: c23-nullptr-assign
 * check-command: sparse -std=c23 -Wno-decl $file
 *
 * check-error-start
c23-nullptr-assign.c:6:12: warning: incorrect type in assignment (different base types)
c23-nullptr-assign.c:6:12:    expected nullptr_t [usertype] np
c23-nullptr-assign.c:6:12:    got int *p
c23-nullptr-assign.c:7:12: warning: incorrect type in assignment (different base types)
c23-nullptr-assign.c:7:12:    expected nullptr_t [usertype] np
c23-nullptr-assign.c:7:12:    got void *v
c23-nullptr-assign.c:8:14: warning: Using plain integer as NULL pointer
c23-nullptr-assign.c:10:17: warning: incorrect type in initializer (different base types)
c23-nullptr-assign.c:10:17:    expected int i
c23-nullptr-assign.c:10:17:    got nullptr_t
c23-nullptr-assign.c:11:20: warning: incorrect type in initializer (different base types)
c23-nullptr-assign.c:11:20:    expected double d
c23-nullptr-assign.c:11:20:    got nullptr_t [assigned] [usertype] np
 * check-error-end
 */
