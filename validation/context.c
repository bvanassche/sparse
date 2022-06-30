#define __cond_lock(c) ((c) ? ({ __context__(1); 1; }) : 0)

static void a(void) __attribute__((context(0,1)))
{
	__context__(1);
}

static void r(void) __attribute__((context(1,0)))
{
	__context__(-1);
}

// Negative output means "conditional positive output"
extern int cond_get(void) __attribute((context(0,-1)));
extern void nonsensical_cond_get(void) __attribute((context(0,-1)));

extern int _ca(int fail);
#define ca(fail) __cond_lock(_ca(fail))

static void good_paired1(void)
{
	a();
	r();
}

static void good_conditional(void)
{
	if (cond_get())
		r();
}

static void nonsensical_conditional(void)
{
	nonsensical_cond_get();
}

static void good_paired2(void)
{
	a();
	r();
	a();
	r();
}

static void good_paired3(void)
{
	a();
	a();
	r();
	r();
}

static void good_lock1(void) __attribute__((context(0,1)))
{
	a();
}

static void good_lock2(void) __attribute__((context(0,1)))
{
	a();
	r();
	a();
}

static void good_lock3(void) __attribute__((context(0,1)))
{
	a();
	a();
	r();
}

static void good_unlock1(void) __attribute__((context(1,0)))
{
	r();
}

static void good_unlock2(void) __attribute__((context(1,0)))
{
	a();
	r();
	r();
}

static void warn_lock1(void)
{
	a();
}

static void warn_lock2(void)
{
	a();
	r();
	a();
}

static void warn_lock3(void)
{
	a();
	a();
	r();
}

static void warn_unlock1(void)
{
	r();
}

static void warn_unlock2(void)
{
	a();
	r();
	r();
}

extern int condition, condition2;

static int good_if1(void)
{
	a();
	if(condition) {
		r();
		return -1;
	}
	r();
	return 0;
}

static void good_if2(void)
{
	if(condition) {
		a();
		r();
	}
}

static void good_if3(void)
{
	a();
	if(condition) {
		a();
		r();
	}
	r();
}

static int warn_if1(void)
{
	a();
	if(condition)
		return -1;
	r();
	return 0;
}

static int warn_if2(void)
{
	a();
	if(condition) {
		r();
		return -1;
	}
	return 0;
}

static void good_while1(void)
{
	a();
	while(condition)
		;
	r();
}

static void good_while2(void)
{
	while(condition) {
		a();
		r();
	}
}

static void good_while3(void)
{
	while(condition) {
		a();
		r();
		if(condition2)
			break;
		a();
		r();
	}
}

static void good_while4(void)
{
	a();
	while(1) {
		if(condition2) {
			r();
			break;
		}
	}
}

static void good_while5(void)
{
	a();
	while(1) {
		r();
		if(condition2)
			break;
		a();
	}
}

static void warn_while1(void)
{
	while(condition) {
		a();
	}
}

static void warn_while2(void)
{
	while(condition) {
		r();
	}
}

static void warn_while3(void)
{
	while(condition) {
		a();
		if(condition2)
			break;
		r();
	}
}

static void good_goto1(void)
{
    a();
    goto label;
label:
    r();
}

static void good_goto2(void)
{
    a();
    goto label;
    a();
    r();
label:
    r();
}

static void good_goto3(void)
{
    a();
    if(condition)
        goto label;
    a();
    r();
label:
    r();
}

static void good_goto4(void)
{
    if(condition)
        goto label;
    a();
    r();
label:
    ;
}

static void good_goto5(void)
{
    a();
    if(condition)
        goto label;
    r();
    return;
label:
    r();
}

static void warn_goto1(void)
{
    a();
    goto label;
    r();
label:
    ;
}

static void warn_goto2(void)
{
    a();
    goto label;
    r();
label:
    a();
    r();
}

static void warn_goto3(void)
{
    a();
    if(condition)
        goto label;
    r();
label:
    r();
}

static void good_cond_lock1(void)
{
    if(ca(condition)) {
        condition2 = 1; /* do stuff */
        r();
    }
}

static void warn_cond_lock1(void)
{
    if(ca(condition))
        condition2 = 1; /* do stuff */
    r();
}

static int good_trylock_def(int ok) __attribute__((context(0, -1)))
{
	if (ok) {
		__context__(1);
		return 1;
	}
	return 0;
}

static int good_trylock_wrap(void) __attribute__((context(0, -1)))
{
	return cond_get();
}

static int good_trylock_stub(void) __attribute__((context(0, -1)))
{
	return 0;
}

extern int multi_cond_get(void)
	__attribute__((context(lock_a, 0, -1)))
	__attribute__((context(lock_b, 0, 1)));
static void release_a(void) __attribute__((context(lock_a, 1, 0)))
{
	__context__(lock_a, -1);
}
static void release_b(void) __attribute__((context(lock_b, 1, 0)))
{
	__context__(lock_b, -1);
}
static void good_multi_cond(void)
{
	if (multi_cond_get())
		release_a();
	release_b();
}

static int warn_trylock_missing(void) __attribute__((context(0, -1)))
{
	return 1;
}

static int warn_trylock_inverted(void) __attribute__((context(0, -1)))
{
	__context__(1);
	return 0;
}

static int warn_trylock_uncond(int x) __attribute__((context(0, -1)))
{
	__context__(1);
	return x;
}
/*
 * check-name: Check -Wcontext
 *
 * check-error-start
context.c:34:29: warning: nonsensical conditional output context with no condition
context.c:84:13: warning: context imbalance in 'warn_lock1' - wrong count at exit
context.c:89:13: warning: context imbalance in 'warn_lock2' - wrong count at exit
context.c:96:13: warning: context imbalance in 'warn_lock3' - wrong count at exit
context.c:103:13: warning: context imbalance in 'warn_unlock1' - unexpected unlock
context.c:108:13: warning: context imbalance in 'warn_unlock2' - unexpected unlock
context.c:146:12: warning: context imbalance in 'warn_if1' - wrong count at exit
context.c:155:12: warning: context imbalance in 'warn_if2' - different lock contexts for basic block
context.c:217:9: warning: context imbalance in 'warn_while1' - different lock contexts for basic block
context.c:225:17: warning: context imbalance in 'warn_while2' - unexpected unlock
context.c:231:9: warning: context imbalance in 'warn_while3' - wrong count at exit
context.c:289:13: warning: context imbalance in 'warn_goto1' - wrong count at exit
context.c:298:13: warning: context imbalance in 'warn_goto2' - wrong count at exit
context.c:315:5: warning: context imbalance in 'warn_goto3' - different lock contexts for basic block
context.c:330:5: warning: context imbalance in 'warn_cond_lock1' - different lock contexts for basic block
context.c:370:12: warning: context imbalance in 'warn_trylock_missing' - wrong count at exit
context.c:375:12: warning: context imbalance in 'warn_trylock_inverted' - wrong count at exit
context.c:381:12: warning: context imbalance in 'warn_trylock_uncond' - wrong count at exit
 * check-error-end
 */
