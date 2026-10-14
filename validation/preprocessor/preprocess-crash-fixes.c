#define REC_A(...) REC_B(__VA_ARGS__)
#define REC_B(...) REC_A(__VA_ARGS__)
extern int REC_A(int, ...);
int rec_nonempty = sizeof(REC_A(1));

/* Recursive macro with empty __VA_ARGS__ must terminate without crash */
#define REC_E(...) REC_F(__VA_ARGS__)
#define REC_F(...) REC_E(__VA_ARGS__)
extern int REC_E(void);
int rec_empty = sizeof(REC_E());

#define ID(x) x
int a = ID(1);

/* Octal escape > 0377 */
int oct_overflow = '\400';

/*
 * check-name: preprocess-crash-fixes
 * check-command: sparse -Wno-decl $file
 *
 * check-error-start
preprocessor/preprocess-crash-fixes.c:16:20: warning: octal escape sequence out of range
 * check-error-end
 */
