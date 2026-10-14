static void lock(void) __attribute__((context(0, 1)))
{
	__context__(1);
}

static void unlock(void) __attribute__((context(1, 0)))
{
	__context__(-1);
}

/* 1. Context attribute before pointer return type */
extern __attribute__((context(0, 1))) void *lock_ret_ptr(void);

static void good_ptr_lock(void)
{
	void *p = lock_ret_ptr();
	(void)p;
	unlock();
}

static void warn_ptr_lock(void)
{
	lock_ret_ptr();
}

/* 2. Unlock followed by lock in same basic block must warn */
static void warn_unlock_then_lock(void)
{
	unlock();
	lock();
}

/*
 * check-name: context-fixes
 * check-command: sparse -Wcontext $file
 *
 * check-error-start
context-fixes.c:21:13: warning: context imbalance in 'warn_ptr_lock' - wrong count at exit
context-fixes.c:27:13: warning: context imbalance in 'warn_unlock_then_lock' - unexpected unlock
 * check-error-end
 */
