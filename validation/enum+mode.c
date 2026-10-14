enum e { ZERO, ONE, TWO };

struct s {
	enum e __attribute__ ((mode(__byte__))) b;
	enum e __attribute__ ((mode(__word__))) w;
	enum e __attribute__ ((mode(__DI__))) d;
	enum e __attribute__ ((mode(__TI__))) t;
};

static struct s s;

_Static_assert(sizeof(s.b) == 1, "");
_Static_assert(sizeof(s.w) == sizeof(long), "");
_Static_assert(sizeof(s.d) == sizeof(long long), "");
_Static_assert(sizeof(s.t) == 16, "");

typedef enum { M_ZERO, M_ONE } __attribute__((mode(__byte__))) byte_enum_t;
_Static_assert(sizeof(byte_enum_t) == 1, "typedef anon enum mode");
_Static_assert((byte_enum_t)-1 > 0, "unsigned enum mode stays unsigned");

typedef enum tagged_m { TM_0, TM_1 } __attribute__((mode(__byte__))) tagged_byte_t;
_Static_assert(sizeof(tagged_byte_t) == 1, "typedef tagged enum mode");
_Static_assert(sizeof(enum tagged_m) == 1, "tagged enum itself gets mode");

enum __attribute__((mode(__HI__))) hi_enum { HI_0 = -1, HI_1 = 1 };
_Static_assert(sizeof(enum hi_enum) == 2, "prefix mode on enum");
_Static_assert((enum hi_enum)-1 < 0, "signed enum mode stays signed");

enum post_qi { PQ_0 = 0, PQ_1 = 200 } __attribute__((mode(__QI__)));
_Static_assert(sizeof(enum post_qi) == 1, "postfix mode on enum");
_Static_assert((enum post_qi)-1 > 0, "postfix QI mode unsigned");

typedef enum { UE_0 = 0, UE_1 = 1 } uenum_t;
static uenum_t __attribute__((mode(__byte__))) ub;
_Static_assert(sizeof(ub) == 1, "typedef var mode size");
_Static_assert((typeof(ub))-1 > 0, "typedef var mode unsigned");

/*
 * check-name: enum+mode
 */
