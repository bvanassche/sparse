static const unsigned char data[] = {
#embed "options.h" limit(1)
};

/*
 * check-name: c23-embed-pedantic
 * check-command: sparse -std=c11 -Wpedantic -I.. $file
 *
 * check-error-start
preprocessor/c23-embed-pedantic.c:2:2: warning: #embed before C23 is an extension
 * check-error-end
 */
