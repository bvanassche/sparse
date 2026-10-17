#ifdef __x86_64__
defined_x86_64
#endif
#ifdef __i386__
defined_i386
#endif

/*
 * check-name: arch/x86-bitness-override
 * check-command: sparse --arch=x86_64 -m16 -D__x86_64__ -E $file
 *
 * check-output-start

defined_x86_64
defined_i386
 * check-output-end
 */
