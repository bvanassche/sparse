static_assert(sizeof(u8'a') == 1, "sizeof u8 char");
static_assert(u8'a' == 97, "value u8 char");
static_assert(u8'\0' == 0, "u8 nul");
static_assert(u8'\xff' == 255, "u8 0xff is unsigned");
static_assert(u8'\u0041' == 65, "u8 UCN ASCII");
static_assert(_Generic(u8'a', unsigned char: 1, char: 0, int: 0, default: 0) == 1,
	      "type u8 char");

static_assert(sizeof(u8"hello") == 6, "sizeof u8 string");
static_assert(sizeof(u8"\u00e9") == 3, "sizeof 2-byte UTF-8 UCN string");
static_assert(sizeof(u8"\U0001F600") == 5, "sizeof 4-byte UTF-8 UCN string");
static_assert(sizeof(u8"ab" "cd") == 5, "concat u8 + narrow");
static_assert(sizeof("ab" u8"cd") == 5, "concat narrow + u8");
static_assert(_Generic(u8"hello", unsigned char *: 1, char *: 0, default: 0) == 1,
	      "u8 string decays to unsigned char *");
static_assert(_Generic("ab" u8"cd", unsigned char *: 1, char *: 0, default: 0) == 1,
	      "narrow + u8 string decays to unsigned char *");

static const unsigned char c = u8'z';
static const unsigned char *s = u8"test";
static char s1[] = u8"abc";
static unsigned char s2[] = u8"abc";
static signed char s3[] = u8"abc";

#define CAT(a, b) a##b
static const unsigned char c2 = CAT(u8, 'x');
static const unsigned char *s_cat = CAT(u8, "yz");

static int use_vars(void)
{
	return c + s[0] + s1[0] + s2[0] + s3[0] + c2 + s_cat[0];
}

/*
 * check-name: c23-u8-char
 * check-command: sparse -std=c23 -pedantic $file
 */
