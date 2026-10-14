struct s_msg {
	char str[16];
	char nonstr[16] __attribute__((__nonstring__));
};

static struct s_msg msg;
static char global_nonstr[32] __attribute__((nonstring));
static char global_str[32];
static char *ptr_nonstr __attribute__((nonstring));
static char *ptr_str;

extern char ext_nonstr[8] __attribute__((nonstring));
char ext_nonstr[8];

_Static_assert(__has_attribute(nonstring), "has_attribute nonstring");
_Static_assert(__has_attribute(__nonstring__), "has_attribute __nonstring__");
_Static_assert(__builtin_has_attribute(msg.nonstr, nonstring), "msg.nonstr has nonstring");
_Static_assert(__builtin_has_attribute(msg.nonstr, __nonstring__), "msg.nonstr has __nonstring__");
_Static_assert(!__builtin_has_attribute(msg.str, nonstring), "msg.str does not have nonstring");
_Static_assert(__builtin_has_attribute(global_nonstr, nonstring), "global_nonstr has nonstring");
_Static_assert(!__builtin_has_attribute(global_str, nonstring), "global_str does not have nonstring");
_Static_assert(__builtin_has_attribute(ptr_nonstr, nonstring), "ptr_nonstr has nonstring");
_Static_assert(!__builtin_has_attribute(ptr_str, nonstring), "ptr_str does not have nonstring");
_Static_assert(__builtin_has_attribute(ext_nonstr, nonstring), "redecl nonstring");

#ifdef __CHECKER__
typedef char nonstr_arr[8] __attribute__((nonstring));
static nonstr_arr *ns_p;
_Static_assert(!__builtin_has_attribute(ns_p, nonstring), "ptr to typedef nonstring does not leak");
#endif

/*
 * check-name: builtin-has-attribute-nonstring
 * check-command: sparse -Wno-decl $file
 */
