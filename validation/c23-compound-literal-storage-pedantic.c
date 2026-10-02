static const int *p = (static int[]){ 1, 2, 3 };

/*
 * check-name: c23-compound-literal-storage-pedantic
 * check-command: sparse -std=c11 -Wpedantic $file
 *
 * check-error-start
c23-compound-literal-storage-pedantic.c:1:24: warning: storage-class specifiers in compound literals are a C23 feature
 * check-error-end
 */
