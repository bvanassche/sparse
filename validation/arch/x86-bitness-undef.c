#ifdef __x86_64__
defined_x86_64
#endif
#ifdef __i386__
defined_i386
#endif

/*
 * check-name: arch/x86-bitness-undef
 * check-command: sparse --arch=x86_64 -D__x86_64__ -m16 -E $file
 *
 * check-output-start

defined_i386
 * check-output-end
 */
