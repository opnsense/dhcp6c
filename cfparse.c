/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20170201

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#define YYPREFIX "yy"

#define YYPURE 0

#line 32 "cfparse.y"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>

/* XXX */

#include <stdio.h>
#include <ctype.h>

#include "dhcp6.h"
#include "config.h"
#include "common.h"

extern int lineno;
extern int cfdebug;
extern int yychar;
extern int yynerrs;

void yywarn(const char *, ...)
	__attribute__((__format__(__printf__, 1, 2)));
void yyerror(const char *, ...)
	__attribute__((__format__(__printf__, 1, 2)));

#define MAKE_NAMELIST(l, n, p) do { \
	(l) = (struct cf_namelist *)malloc(sizeof(*(l))); \
	if ((l) == NULL) { \
		yywarn("can't allocate memory"); \
		if (p) cleanup_cflist(p); \
		return (-1); \
	} \
	memset((l), 0, sizeof(*(l))); \
	l->line = lineno; \
	l->name = (n); \
	l->params = (p); \
	} while (0)

#define MAKE_CFLIST(l, t, pp, pl) do { \
	(l) = (struct cf_list *)malloc(sizeof(*(l))); \
	if ((l) == NULL) { \
		yywarn("can't allocate memory"); \
		if (pp) free(pp); \
		if (pl) cleanup_cflist(pl); \
		return (-1); \
	} \
	memset((l), 0, sizeof(*(l))); \
	l->line = lineno; \
	l->type = (t); \
	l->ptr = (pp); \
	l->list = (pl); \
	l->tail = (l); \
	} while (0)

static struct cf_namelist *iflist_head, *hostlist_head, *iapdlist_head;
static struct cf_namelist *addrpoollist_head;
static struct cf_namelist *authinfolist_head, *keylist_head;
static struct cf_namelist *ianalist_head;

extern struct cf_list *cf_dns_list, *cf_dns_name_list, *cf_ntp_list;
extern struct cf_list *cf_sip_list, *cf_sip_name_list;
extern struct cf_list *cf_nis_list, *cf_nis_name_list;
extern struct cf_list *cf_nisp_list, *cf_nisp_name_list;
extern struct cf_list *cf_bcmcs_list, *cf_bcmcs_name_list;
struct cf_list *cf_dns_list, *cf_dns_name_list, *cf_ntp_list;
struct cf_list *cf_sip_list, *cf_sip_name_list;
struct cf_list *cf_nis_list, *cf_nis_name_list;
struct cf_list *cf_nisp_list, *cf_nisp_name_list;
struct cf_list *cf_bcmcs_list, *cf_bcmcs_name_list;

extern long long cf_refreshtime;
long long cf_refreshtime = -1;

int yylex(void);
int cfswitch_buffer(char *);
static int add_namelist(struct cf_namelist *, struct cf_namelist **);
static void cleanup(void);
static void cleanup_namelist(struct cf_namelist *);
static void cleanup_cflist(struct cf_list *);
int cf_post_config(void);
void cf_init(void);
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#line 143 "cfparse.y"
typedef union {
	long long num;
	char* str;
	struct cf_list *list;
	struct dhcp6_prefix *prefix;
	struct dhcp6_range *range;
	struct dhcp6_poolspec *pool;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 125 "y.tab.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define INTERFACE 257
#define IFNAME 258
#define PREFIX_INTERFACE 259
#define SLA_ID 260
#define SLA_LEN 261
#define DUID_ID 262
#define ID_ASSOC 263
#define IA_PD 264
#define IAID 265
#define IA_NA 266
#define ADDRESS 267
#define REQUEST 268
#define SEND 269
#define ALLOW 270
#define PREFERENCE 271
#define HOST 272
#define HOSTNAME 273
#define DUID 274
#define OPTION 275
#define RAPID_COMMIT 276
#define DNS_SERVERS 277
#define DNS_NAME 278
#define NTP_SERVERS 279
#define REFRESHTIME 280
#define SIP_SERVERS 281
#define SIP_NAME 282
#define NIS_SERVERS 283
#define NIS_NAME 284
#define NISP_SERVERS 285
#define NISP_NAME 286
#define BCMCS_SERVERS 287
#define BCMCS_NAME 288
#define INFO_ONLY 289
#define SCRIPT 290
#define DELAYEDKEY 291
#define AUTHENTICATION 292
#define PROTOCOL 293
#define ALGORITHM 294
#define DELAYED 295
#define RECONFIG 296
#define HMACMD5 297
#define MONOCOUNTER 298
#define AUTHNAME 299
#define RDM 300
#define KEY 301
#define KEYINFO 302
#define REALM 303
#define KEYID 304
#define SECRET 305
#define KEYNAME 306
#define EXPIRE 307
#define ADDRPOOL 308
#define POOLNAME 309
#define RANGE 310
#define TO 311
#define ADDRESS_POOL 312
#define INCLUDE 313
#define NUMBER 314
#define SLASH 315
#define EOS 316
#define BCL 317
#define ECL 318
#define STRING 319
#define QSTRING 320
#define PREFIX 321
#define INFINITY 322
#define COMMA 323
#define RAW 324
#define YYERRCODE 256
typedef int YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,   26,   26,   26,   26,   26,   26,   26,   26,
   27,   28,   29,   29,   29,   29,   29,   29,   29,   29,
   29,   29,   29,   29,   30,   30,   30,   30,   31,   32,
   34,   33,   10,   10,   11,    6,    6,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,    5,   12,
   12,    7,    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,   24,   22,
   22,   23,   23,   25,   25,    1,    1,   13,   13,   14,
   14,   15,    9,    9,    8,    8,   16,   16,   17,   18,
   18,   19,   19,   19,   19,    2,    2,    3,    4,   20,
   20,   21,   21,   21,   21,
};
static const YYINT yylen[] = {                            2,
    0,    2,    1,    1,    1,    1,    1,    1,    1,    1,
    6,    6,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    7,    6,    7,    6,    6,    6,
    3,    6,    0,    2,    1,    0,    2,    3,    3,    2,
    3,    3,    3,    3,    3,    3,    3,    3,    3,    1,
    3,    1,    2,    2,    2,    1,    1,    3,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    3,    2,
    3,    4,    5,    2,    3,    1,    1,    0,    2,    1,
    3,    6,    0,    2,    3,    3,    0,    2,    3,    0,
    2,    3,    3,    3,    3,    1,    1,    1,    1,    0,
    2,    3,    3,    3,    3,
};
static const YYINT yydefred[] = {                         1,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,
    3,    4,    5,    6,    7,    8,    9,   10,    0,    0,
    0,    0,   33,    0,   33,    0,   33,    0,   33,    0,
   33,    0,   33,    0,    0,    0,    0,    0,   36,    0,
   78,    0,   87,   36,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   90,  100,   36,   31,
    0,   78,    0,   87,    0,    0,   13,   35,   34,   14,
   15,   24,   16,   17,   18,   19,   20,   21,   22,   23,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   37,    0,    0,    0,
    0,   79,   80,    0,    0,    0,   88,    0,    0,    0,
    0,    0,    0,   91,    0,    0,    0,    0,    0,  101,
    0,    0,    0,    0,    0,   52,   59,   60,   61,   62,
   56,   57,   63,   64,   65,   66,   67,   68,    0,    0,
    0,    0,    0,    0,    0,    0,   40,    0,    0,    0,
    0,    0,    0,   11,    0,    0,    0,    0,   26,    0,
    0,    0,   28,   12,   96,   97,    0,   98,    0,   99,
    0,    0,   29,    0,    0,    0,    0,   30,   32,   77,
   76,    0,   43,   54,   55,   53,    0,    0,   39,   38,
   41,   45,   42,   46,   47,    0,   48,    0,   49,    0,
   44,   25,   83,   81,   27,   89,   92,   93,   94,   95,
  102,  103,  104,  105,   71,   58,   51,   69,   75,    0,
    0,    0,    0,    0,    0,   84,   73,    0,    0,   82,
   85,   86,
};
static const YYINT yydgoto[] = {                          1,
  182,  167,  169,  171,   97,   61,  141,  226,  221,   45,
   69,  142,   63,  102,  103,   65,  107,   81,  114,   82,
  120,  123,  156,  151,  153,   10,   11,   12,   13,   14,
   15,   16,   17,   18,
};
static const YYINT yysindex[] = {                         0,
 -224, -243, -172, -266, -105, -262, -265, -264, -248,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -267, -260,
 -254, -240,    0, -229,    0, -218,    0, -220,    0, -219,
    0, -216,    0, -208, -204, -202, -201, -231,    0, -195,
    0, -194,    0,    0, -303, -191, -302, -190, -270, -189,
 -246, -188, -237, -181, -226, -179,    0,    0,    0,    0,
 -215,    0, -257,    0, -235, -160,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -213, -171, -150, -259, -258, -258, -258, -176, -120, -173,
 -175, -170, -168, -166, -162, -164,    0, -256, -114, -159,
 -164,    0,    0, -232, -259, -157,    0, -153, -197, -149,
 -152, -163, -147,    0, -156, -148, -155, -136, -146,    0,
 -131, -310, -130, -127, -126,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -132, -125,
 -133, -124, -123, -122, -121, -119,    0, -118, -117, -115,
 -116, -310, -113,    0, -111, -110, -109, -112,    0, -108,
 -107, -106,    0,    0,    0,    0, -104,    0, -103,    0,
 -102, -101,    0, -100,  -99,  -98,  -97,    0,    0,    0,
    0, -310,    0,    0,    0,    0, -128, -258,    0,    0,
    0,    0,    0,    0,    0,  -96,    0, -310,    0,  -94,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -310,
 -251, -310,  -93,  -92,  -91,    0,    0,  -90,  -89,    0,
    0,    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -88,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -191,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -189,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -190,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
static const YYINT yygindex[] = {                         0,
 -151,    0,    0,    0,    0,   -1,  114,    0,    0,   11,
    0,  -86,  140,    0,    0,  147,    0,    0,    0,    0,
    0,  119,  128,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
#define YYTABLESIZE 229
static const YYINT yytable[] = {                        143,
  198,   99,   99,  180,   40,  124,   22,  125,  223,  224,
   42,  181,   67,   71,   19,   68,   68,  126,  127,  128,
  129,  130,  131,  132,  133,  134,  135,  136,  137,  138,
  215,  105,    2,  139,  105,   47,   35,   49,    3,   51,
   36,   53,   66,   55,   37,   73,  219,    4,   68,   39,
    5,   84,   85,   86,   87,   88,   41,   83,   89,  122,
  100,  157,   43,  101,  101,  140,  225,    6,  222,   75,
  227,   38,   68,   90,   91,   92,   44,    7,   77,  109,
  110,   68,  106,    8,   60,  161,  111,  112,    9,   79,
   46,   20,   68,   21,   93,   48,   94,  165,  166,   50,
   52,  217,   95,   54,  113,   96,   84,   85,   86,   87,
   88,   56,   57,   89,   58,   59,   84,   85,   86,   87,
   88,   62,   64,   89,   70,   72,   74,   76,   90,   91,
   92,  115,  116,  117,   78,  118,   80,  145,   90,   91,
   92,  146,  147,  158,  148,  170,  119,  168,  149,   93,
  150,   94,  152,  154,  155,  172,  159,  108,  163,   93,
   96,   94,  164,  174,  176,  175,  186,  121,  173,  178,
   96,   23,   24,   25,   26,   27,   28,   29,   30,   31,
   32,   33,   34,  177,  179,  183,  184,  185,  187,  188,
  216,  189,  190,  191,  192,  196,  193,  194,  195,  197,
  144,   98,  199,  200,  203,  201,  202,  204,  205,  206,
  104,  207,  208,  209,  210,  211,  212,  213,  214,  220,
  228,  229,  218,  162,  230,  231,  232,   50,  160,
};
static const YYINT yycheck[] = {                         86,
  152,  259,  259,  314,  265,  264,  273,  266,  260,  261,
  265,  322,  316,  316,  258,  319,  319,  276,  277,  278,
  279,  280,  281,  282,  283,  284,  285,  286,  287,  288,
  182,  267,  257,  292,  267,   25,  299,   27,  263,   29,
  306,   31,   44,   33,  309,  316,  198,  272,  319,  317,
  275,  267,  268,  269,  270,  271,  317,   59,  274,  319,
  318,  318,  317,  321,  321,  324,  318,  292,  220,  316,
  222,  320,  319,  289,  290,  291,  317,  302,  316,  293,
  294,  319,  318,  308,  316,  318,  300,  301,  313,  316,
  320,  264,  319,  266,  310,  314,  312,  295,  296,  320,
  320,  188,  318,  320,  318,  321,  267,  268,  269,  270,
  271,  320,  317,  274,  317,  317,  267,  268,  269,  270,
  271,  317,  317,  274,  316,  316,  316,  316,  289,  290,
  291,  303,  304,  305,  316,  307,  316,  314,  289,  290,
  291,  262,  316,  258,  320,  298,  318,  297,  319,  310,
  319,  312,  319,  316,  319,  319,  316,  318,  316,  310,
  321,  312,  316,  320,  320,  314,  299,  318,  316,  316,
  321,  277,  278,  279,  280,  281,  282,  283,  284,  285,
  286,  287,  288,  320,  316,  316,  314,  314,  314,  323,
  319,  316,  316,  316,  316,  311,  316,  316,  316,  316,
   87,   62,  316,  315,  317,  316,  316,  316,  316,  316,
   64,  316,  316,  316,  316,  316,  316,  316,  316,  314,
  314,  314,  319,  105,  316,  316,  316,  316,  101,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 324
#define YYUNDFTOKEN 361
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTERFACE","IFNAME",
"PREFIX_INTERFACE","SLA_ID","SLA_LEN","DUID_ID","ID_ASSOC","IA_PD","IAID",
"IA_NA","ADDRESS","REQUEST","SEND","ALLOW","PREFERENCE","HOST","HOSTNAME",
"DUID","OPTION","RAPID_COMMIT","DNS_SERVERS","DNS_NAME","NTP_SERVERS",
"REFRESHTIME","SIP_SERVERS","SIP_NAME","NIS_SERVERS","NIS_NAME","NISP_SERVERS",
"NISP_NAME","BCMCS_SERVERS","BCMCS_NAME","INFO_ONLY","SCRIPT","DELAYEDKEY",
"AUTHENTICATION","PROTOCOL","ALGORITHM","DELAYED","RECONFIG","HMACMD5",
"MONOCOUNTER","AUTHNAME","RDM","KEY","KEYINFO","REALM","KEYID","SECRET",
"KEYNAME","EXPIRE","ADDRPOOL","POOLNAME","RANGE","TO","ADDRESS_POOL","INCLUDE",
"NUMBER","SLASH","EOS","BCL","ECL","STRING","QSTRING","PREFIX","INFINITY",
"COMMA","RAW",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : statements",
"statements :",
"statements : statements statement",
"statement : interface_statement",
"statement : host_statement",
"statement : option_statement",
"statement : ia_statement",
"statement : authentication_statement",
"statement : key_statement",
"statement : addrpool_statement",
"statement : include_statement",
"interface_statement : INTERFACE IFNAME BCL declarations ECL EOS",
"host_statement : HOST HOSTNAME BCL declarations ECL EOS",
"option_statement : OPTION DNS_SERVERS address_list EOS",
"option_statement : OPTION DNS_NAME QSTRING EOS",
"option_statement : OPTION NTP_SERVERS address_list EOS",
"option_statement : OPTION SIP_SERVERS address_list EOS",
"option_statement : OPTION SIP_NAME QSTRING EOS",
"option_statement : OPTION NIS_SERVERS address_list EOS",
"option_statement : OPTION NIS_NAME QSTRING EOS",
"option_statement : OPTION NISP_SERVERS address_list EOS",
"option_statement : OPTION NISP_NAME QSTRING EOS",
"option_statement : OPTION BCMCS_SERVERS address_list EOS",
"option_statement : OPTION BCMCS_NAME QSTRING EOS",
"option_statement : OPTION REFRESHTIME NUMBER EOS",
"ia_statement : ID_ASSOC IA_PD IAID BCL iapdconf_list ECL EOS",
"ia_statement : ID_ASSOC IA_PD BCL iapdconf_list ECL EOS",
"ia_statement : ID_ASSOC IA_NA IAID BCL ianaconf_list ECL EOS",
"ia_statement : ID_ASSOC IA_NA BCL ianaconf_list ECL EOS",
"authentication_statement : AUTHENTICATION AUTHNAME BCL authparam_list ECL EOS",
"key_statement : KEYINFO KEYNAME BCL keyparam_list ECL EOS",
"include_statement : INCLUDE QSTRING EOS",
"addrpool_statement : ADDRPOOL POOLNAME BCL declarations ECL EOS",
"address_list :",
"address_list : address_list address_list_ent",
"address_list_ent : STRING",
"declarations :",
"declarations : declarations declaration",
"declaration : SEND dhcpoption_list EOS",
"declaration : REQUEST dhcpoption_list EOS",
"declaration : INFO_ONLY EOS",
"declaration : ALLOW dhcpoption EOS",
"declaration : DUID DUID_ID EOS",
"declaration : ADDRESS addressparam EOS",
"declaration : PREFIX prefixparam EOS",
"declaration : PREFERENCE NUMBER EOS",
"declaration : SCRIPT QSTRING EOS",
"declaration : DELAYEDKEY STRING EOS",
"declaration : RANGE rangeparam EOS",
"declaration : ADDRESS_POOL poolparam EOS",
"dhcpoption_list : dhcpoption",
"dhcpoption_list : dhcpoption COMMA dhcpoption_list",
"dhcpoption : RAPID_COMMIT",
"dhcpoption : AUTHENTICATION AUTHNAME",
"dhcpoption : IA_PD NUMBER",
"dhcpoption : IA_NA NUMBER",
"dhcpoption : SIP_SERVERS",
"dhcpoption : SIP_NAME",
"dhcpoption : RAW NUMBER STRING",
"dhcpoption : DNS_SERVERS",
"dhcpoption : DNS_NAME",
"dhcpoption : NTP_SERVERS",
"dhcpoption : REFRESHTIME",
"dhcpoption : NIS_SERVERS",
"dhcpoption : NIS_NAME",
"dhcpoption : NISP_SERVERS",
"dhcpoption : NISP_NAME",
"dhcpoption : BCMCS_SERVERS",
"dhcpoption : BCMCS_NAME",
"rangeparam : STRING TO STRING",
"addressparam : STRING duration",
"addressparam : STRING duration duration",
"prefixparam : STRING SLASH NUMBER duration",
"prefixparam : STRING SLASH NUMBER duration duration",
"poolparam : STRING duration",
"poolparam : STRING duration duration",
"duration : INFINITY",
"duration : NUMBER",
"iapdconf_list :",
"iapdconf_list : iapdconf_list iapdconf",
"iapdconf : prefix_interface",
"iapdconf : PREFIX prefixparam EOS",
"prefix_interface : PREFIX_INTERFACE IFNAME BCL ifparams ECL EOS",
"ifparams :",
"ifparams : ifparams ifparam",
"ifparam : SLA_ID NUMBER EOS",
"ifparam : SLA_LEN NUMBER EOS",
"ianaconf_list :",
"ianaconf_list : ianaconf_list ianaconf",
"ianaconf : ADDRESS addressparam EOS",
"authparam_list :",
"authparam_list : authparam_list authparam",
"authparam : PROTOCOL authproto EOS",
"authparam : ALGORITHM authalg EOS",
"authparam : RDM authrdm EOS",
"authparam : KEY STRING EOS",
"authproto : DELAYED",
"authproto : RECONFIG",
"authalg : HMACMD5",
"authrdm : MONOCOUNTER",
"keyparam_list :",
"keyparam_list : keyparam_list keyparam",
"keyparam : REALM QSTRING EOS",
"keyparam : KEYID NUMBER EOS",
"keyparam : SECRET QSTRING EOS",
"keyparam : EXPIRE QSTRING EOS",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 1275 "cfparse.y"
/* supplement routines for configuration */
static int
add_namelist(struct cf_namelist *new, struct cf_namelist **headp)
{
	struct cf_namelist *n;

	/* check for duplicated configuration */
	for (n = *headp; n; n = n->next) {
		if (strcmp(n->name, new->name) == 0) {
			yywarn("duplicated name: %s (ignored)",
			       new->name);
			cleanup_namelist(new);
			return (0);
		}
	}

	new->next = *headp;
	*headp = new;

	return (0);
}

/* free temporary resources */
static void
cleanup(void)
{
	cleanup_namelist(iflist_head);
	iflist_head = NULL;
	cleanup_namelist(hostlist_head);
	hostlist_head = NULL;
	cleanup_namelist(iapdlist_head);
	iapdlist_head = NULL;
	cleanup_namelist(ianalist_head);
	ianalist_head = NULL;
	cleanup_namelist(authinfolist_head);
	authinfolist_head = NULL;
	cleanup_namelist(keylist_head);
	keylist_head = NULL;
	cleanup_namelist(addrpoollist_head);
	addrpoollist_head = NULL;

	cleanup_cflist(cf_sip_list);
	cf_sip_list = NULL;
	cleanup_cflist(cf_sip_name_list);
	cf_sip_name_list = NULL;
	cleanup_cflist(cf_dns_list);
	cf_dns_list = NULL;
	cleanup_cflist(cf_dns_name_list);
	cf_dns_name_list = NULL;
	cleanup_cflist(cf_ntp_list);
	cf_ntp_list = NULL;
	cleanup_cflist(cf_nis_list);
	cf_nis_list = NULL;
	cleanup_cflist(cf_nis_name_list);
	cf_nis_name_list = NULL;
	cleanup_cflist(cf_nisp_list);
	cf_nisp_list = NULL;
	cleanup_cflist(cf_nisp_name_list);
	cf_nisp_name_list = NULL;
	cleanup_cflist(cf_bcmcs_list);
	cf_bcmcs_list = NULL;
	cleanup_cflist(cf_bcmcs_name_list);
	cf_bcmcs_name_list = NULL;
}

static void
cleanup_namelist(struct cf_namelist *head)
{
	struct cf_namelist *ifp, *ifp_next;

	for (ifp = head; ifp; ifp = ifp_next) {
		ifp_next = ifp->next;
		cleanup_cflist(ifp->params);
		free(ifp->name);
		free(ifp);
	}
}

static void
cleanup_cflist(struct cf_list *p)
{
	struct cf_list *n;

	if (p == NULL)
		return;

	n = p->next;
	if (p->type == DECL_ADDRESSPOOL) {
		free(((struct dhcp6_poolspec *)p->ptr)->name);
	}
	if (p->ptr)
		free(p->ptr);
	if (p->list)
		cleanup_cflist(p->list);
	free(p);

	cleanup_cflist(n);
}

#define config_fail() \
	do { cleanup(); configure_cleanup(); return (-1); } while(0)

int
cf_post_config(void)
{
	if (configure_keys(keylist_head))
		config_fail();

	if (configure_authinfo(authinfolist_head))
		config_fail();

	if (configure_ia(iapdlist_head, IATYPE_PD))
		config_fail();

	if (configure_ia(ianalist_head, IATYPE_NA))
		config_fail();

	if (configure_pool(addrpoollist_head))
		config_fail();

	if (configure_interface(iflist_head))
		config_fail();

	if (configure_host(hostlist_head))
		config_fail();

	if (configure_global_option())
		config_fail();

	configure_commit();
	cleanup();
	return (0);
}
#undef config_fail

void
cf_init(void)
{
	iflist_head = NULL;
}
#line 709 "y.tab.c"

#if YYDEBUG
#include <stdio.h>	/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yym = 0;
    yyn = 0;
    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        yychar = YYLEX;
        if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if (((yyn = yysindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if (((yyn = yyrindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag != 0) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab; /* redundant goto avoids 'unused label' warning */
yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if (((yyn = yysindex[*yystack.s_mark]) != 0) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym > 0)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);

    switch (yyn)
    {
case 11:
#line 187 "cfparse.y"
	{
		struct cf_namelist *ifl;

		MAKE_NAMELIST(ifl, yystack.l_mark[-4].str, yystack.l_mark[-2].list);

		if (add_namelist(ifl, &iflist_head))
			return (-1);
	}
break;
case 12:
#line 199 "cfparse.y"
	{
		struct cf_namelist *host;

		MAKE_NAMELIST(host, yystack.l_mark[-4].str, yystack.l_mark[-2].list);

		if (add_namelist(host, &hostlist_head))
			return (-1);
	}
break;
case 13:
#line 211 "cfparse.y"
	{
			if (cf_dns_list == NULL)
				cf_dns_list = yystack.l_mark[-1].list;
			else {
				cf_dns_list->tail->next = yystack.l_mark[-1].list;
				cf_dns_list->tail = yystack.l_mark[-1].list->tail;
			}
		}
break;
case 14:
#line 220 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, CFLISTENT_GENERIC, yystack.l_mark[-1].str, NULL);

			if (cf_dns_name_list == NULL) {
				cf_dns_name_list = l;
				cf_dns_name_list->tail = l;
				cf_dns_name_list->next = NULL;
			} else {
				cf_dns_name_list->tail->next = l;
				cf_dns_name_list->tail = l->tail;
			}
		}
break;
case 15:
#line 235 "cfparse.y"
	{
			if (cf_ntp_list == NULL)
				cf_ntp_list = yystack.l_mark[-1].list;
			else {
				cf_ntp_list->tail->next = yystack.l_mark[-1].list;
				cf_ntp_list->tail = yystack.l_mark[-1].list->tail;
			}
		}
break;
case 16:
#line 244 "cfparse.y"
	{
			if (cf_sip_list == NULL)
				cf_sip_list = yystack.l_mark[-1].list;
			else {
				cf_sip_list->tail->next = yystack.l_mark[-1].list;
				cf_sip_list->tail = yystack.l_mark[-1].list->tail;
			}
		}
break;
case 17:
#line 253 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, CFLISTENT_GENERIC, yystack.l_mark[-1].str, NULL);

			if (cf_sip_name_list == NULL) {
				cf_sip_name_list = l;
				cf_sip_name_list->tail = l;
				cf_sip_name_list->next = NULL;
			} else {
				cf_sip_name_list->tail->next = l;
				cf_sip_name_list->tail = l->tail;
			}
		}
break;
case 18:
#line 268 "cfparse.y"
	{
			if (cf_nis_list == NULL)
				cf_nis_list = yystack.l_mark[-1].list;
			else {
				cf_nis_list->tail->next = yystack.l_mark[-1].list;
				cf_nis_list->tail = yystack.l_mark[-1].list->tail;
			}
		}
break;
case 19:
#line 277 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, CFLISTENT_GENERIC, yystack.l_mark[-1].str, NULL);

			if (cf_nis_name_list == NULL) {
				cf_nis_name_list = l;
				cf_nis_name_list->tail = l;
				cf_nis_name_list->next = NULL;
			} else {
				cf_nis_name_list->tail->next = l;
				cf_nis_name_list->tail = l->tail;
			}
		}
break;
case 20:
#line 292 "cfparse.y"
	{
			if (cf_nisp_list == NULL)
				cf_nisp_list = yystack.l_mark[-1].list;
			else {
				cf_nisp_list->tail->next = yystack.l_mark[-1].list;
				cf_nisp_list->tail = yystack.l_mark[-1].list->tail;
			}
		}
break;
case 21:
#line 301 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, CFLISTENT_GENERIC, yystack.l_mark[-1].str, NULL);

			if (cf_nisp_name_list == NULL) {
				cf_nisp_name_list = l;
				cf_nisp_name_list->tail = l;
				cf_nisp_name_list->next = NULL;
			} else {
				cf_nisp_name_list->tail->next = l;
				cf_nisp_name_list->tail = l->tail;
			}
		}
break;
case 22:
#line 316 "cfparse.y"
	{
			if (cf_bcmcs_list == NULL)
				cf_bcmcs_list = yystack.l_mark[-1].list;
			else {
				cf_bcmcs_list->tail->next = yystack.l_mark[-1].list;
				cf_bcmcs_list->tail = yystack.l_mark[-1].list->tail;
			}
		}
break;
case 23:
#line 325 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, CFLISTENT_GENERIC, yystack.l_mark[-1].str, NULL);

			if (cf_bcmcs_name_list == NULL) {
				cf_bcmcs_name_list = l;
				cf_bcmcs_name_list->tail = l;
				cf_bcmcs_name_list->next = NULL;
			} else {
				cf_bcmcs_name_list->tail->next = l;
				cf_bcmcs_name_list->tail = l->tail;
			}
		}
break;
case 24:
#line 340 "cfparse.y"
	{
			if (cf_refreshtime == -1) {
				cf_refreshtime = yystack.l_mark[-1].num;
				if (cf_refreshtime < -1 ||
				    cf_refreshtime > 0xffffffff) {
					/*
					 * refresh time should not be negative
					 * according to the lex definition,
					 * but check it for safety.
					 */
					yyerror("refresh time is out of range");
				}
				if (cf_refreshtime < DHCP6_IRT_MINIMUM) {
					/*
					 * the value MUST NOT be smaller than
					 * IRT_MINIMUM.
					 */
					yyerror("refresh time is too small "
					    "(must not be smaller than %d)",
					    DHCP6_IRT_MINIMUM);
				}
			} else {
				yywarn("multiple refresh times (ignored)");
			}
		}
break;
case 25:
#line 369 "cfparse.y"
	{
			struct cf_namelist *iapd;

			MAKE_NAMELIST(iapd, yystack.l_mark[-4].str, yystack.l_mark[-2].list);

			if (add_namelist(iapd, &iapdlist_head))
				return (-1);
		}
break;
case 26:
#line 378 "cfparse.y"
	{
			struct cf_namelist *iapd;
			char *zero;

			if ((zero = strdup("0")) == NULL) {
				yywarn("can't allocate memory");
				return (-1);
			}
			MAKE_NAMELIST(iapd, zero, yystack.l_mark[-2].list);

			if (add_namelist(iapd, &iapdlist_head))
				return (-1);
		}
break;
case 27:
#line 392 "cfparse.y"
	{
			struct cf_namelist *iana;

			MAKE_NAMELIST(iana, yystack.l_mark[-4].str, yystack.l_mark[-2].list);

			if (add_namelist(iana, &ianalist_head))
				return (-1);
		}
break;
case 28:
#line 401 "cfparse.y"
	{
			struct cf_namelist *iana;
			char *zero;

			if ((zero = strdup("0")) == NULL) {
				yywarn("can't allocate memory");
				return (-1);
			}
			MAKE_NAMELIST(iana, zero, yystack.l_mark[-2].list);

			if (add_namelist(iana, &ianalist_head))
				return (-1);
		}
break;
case 29:
#line 418 "cfparse.y"
	{
		struct cf_namelist *authinfo;

		MAKE_NAMELIST(authinfo, yystack.l_mark[-4].str, yystack.l_mark[-2].list);

		if (add_namelist(authinfo, &authinfolist_head))
			return (-1);
	}
break;
case 30:
#line 430 "cfparse.y"
	{
		struct cf_namelist *key;

		MAKE_NAMELIST(key, yystack.l_mark[-4].str, yystack.l_mark[-2].list);

		if (add_namelist(key, &keylist_head))
			return (-1);
	}
break;
case 31:
#line 442 "cfparse.y"
	{
		if (cfswitch_buffer(yystack.l_mark[-1].str)) {
			free(yystack.l_mark[-1].str);
			return (-1);
		}
		free(yystack.l_mark[-1].str);
	}
break;
case 32:
#line 453 "cfparse.y"
	{
		struct cf_namelist *pool;

		MAKE_NAMELIST(pool, yystack.l_mark[-4].str, yystack.l_mark[-2].list);

		if (add_namelist(pool, &addrpoollist_head))
			return (-1);
	}
break;
case 33:
#line 464 "cfparse.y"
	{ yyval.list = NULL; }
break;
case 34:
#line 466 "cfparse.y"
	{
			struct cf_list *head;

			if ((head = yystack.l_mark[-1].list) == NULL) {
				yystack.l_mark[0].list->next = NULL;
				yystack.l_mark[0].list->tail = yystack.l_mark[0].list;
				head = yystack.l_mark[0].list;
			} else {
				head->tail->next = yystack.l_mark[0].list;
				head->tail = yystack.l_mark[0].list->tail;
			}

			yyval.list = head;
		}
break;
case 35:
#line 484 "cfparse.y"
	{
		struct cf_list *l;
		struct in6_addr a0, *a;

		if (inet_pton(AF_INET6, yystack.l_mark[0].str, &a0) != 1) {
			yywarn("invalid IPv6 address: %s", yystack.l_mark[0].str);
			free(yystack.l_mark[0].str);
			return (-1);
		}
		if ((a = malloc(sizeof(*a))) == NULL) {
			yywarn("can't allocate memory");
			return (-1);
		}
		*a = a0;

		MAKE_CFLIST(l, CFLISTENT_GENERIC, a, NULL);

		yyval.list = l;
	}
break;
case 36:
#line 506 "cfparse.y"
	{ yyval.list = NULL; }
break;
case 37:
#line 508 "cfparse.y"
	{
			struct cf_list *head;

			if ((head = yystack.l_mark[-1].list) == NULL) {
				yystack.l_mark[0].list->next = NULL;
				yystack.l_mark[0].list->tail = yystack.l_mark[0].list;
				head = yystack.l_mark[0].list;
			} else {
				head->tail->next = yystack.l_mark[0].list;
				head->tail = yystack.l_mark[0].list->tail;
			}

			yyval.list = head;
		}
break;
case 38:
#line 526 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_SEND, NULL, yystack.l_mark[-1].list);

			yyval.list = l;
		}
break;
case 39:
#line 534 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_REQUEST, NULL, yystack.l_mark[-1].list);

			yyval.list = l;
		}
break;
case 40:
#line 542 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_INFO_ONLY, NULL, NULL);
			/* no value */
			yyval.list = l;
		}
break;
case 41:
#line 550 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_ALLOW, NULL, yystack.l_mark[-1].list);

			yyval.list = l;
		}
break;
case 42:
#line 558 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_DUID, yystack.l_mark[-1].str, NULL);

			yyval.list = l;
		}
break;
case 43:
#line 566 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_ADDRESS, yystack.l_mark[-1].prefix,NULL);

			yyval.list = l;
		}
break;
case 44:
#line 574 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_PREFIX, yystack.l_mark[-1].prefix, NULL);

			yyval.list = l;
		}
break;
case 45:
#line 582 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_PREFERENCE, NULL, NULL);
			l->num = yystack.l_mark[-1].num;

			yyval.list = l;
		}
break;
case 46:
#line 591 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_SCRIPT, yystack.l_mark[-1].str, NULL);

			yyval.list = l;
		}
break;
case 47:
#line 599 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_DELAYEDKEY, yystack.l_mark[-1].str, NULL);

			yyval.list = l;
		}
break;
case 48:
#line 607 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_RANGE, yystack.l_mark[-1].range, NULL);

			yyval.list = l;
		}
break;
case 49:
#line 615 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DECL_ADDRESSPOOL, yystack.l_mark[-1].pool, NULL);

			yyval.list = l;
		}
break;
case 50:
#line 626 "cfparse.y"
	{
			yyval.list = yystack.l_mark[0].list;
		}
break;
case 51:
#line 630 "cfparse.y"
	{
			yystack.l_mark[-2].list->next = yystack.l_mark[0].list;
			yystack.l_mark[-2].list->tail = yystack.l_mark[0].list->tail;

			yyval.list = yystack.l_mark[-2].list;
		}
break;
case 52:
#line 640 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_RAPID_COMMIT, NULL, NULL);
			/* no value */
			yyval.list = l;
		}
break;
case 53:
#line 648 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_AUTHINFO, NULL, NULL);
			l->ptr = yystack.l_mark[0].str;
			yyval.list = l;
		}
break;
case 54:
#line 656 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_IA_PD, NULL, NULL);
			l->num = yystack.l_mark[0].num;
			yyval.list = l;
		}
break;
case 55:
#line 664 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_IA_NA, NULL, NULL);
			l->num = yystack.l_mark[0].num;
			yyval.list = l;
		}
break;
case 56:
#line 672 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_SIP, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 57:
#line 680 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_SIPNAME, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 58:
#line 689 "cfparse.y"
	{
			struct cf_list *l;
			struct rawoption *rawop;
			char *tmp, *opstr = yystack.l_mark[-1].num, *datastr = yystack.l_mark[0].str;

			yywarn("Got raw option: %s %s", opstr, datastr);

			if ((rawop = malloc(sizeof(*rawop))) == NULL) {
				yywarn("can't allocate memory");
				free(datastr);
				free(opstr);
				return (-1);
			}

			/* convert op num */
			rawop->opnum = (int)strtol(opstr, NULL, 10);

			/* convert string to lowercase */
			tmp = datastr;
			for ( ; *tmp; ++tmp) *tmp = tolower(*tmp);

			/* allocate buffer */
			int len = strlen(datastr);
			len -= len / 3; /* remove ':' from length */
			len = len / 2; /* byte length */
			rawop->datalen = len;

			if ((rawop->data = malloc(len)) == NULL) {
				yywarn("can't allocate memory");
				free(datastr);
				free(opstr);
				return (-1);
			}

			/* convert hex string to byte array */
			char *h = datastr;
			char *b = rawop->data;
			char xlate[] = "0123456789abcdef";
			int p1, p2, i = 0;

			for ( ; *h; h += 3, ++b) { /* string is xx(:xx)\0 */
				p1 = (int)(strchr(xlate, *h) - xlate);
				p2 = (int)(strchr(xlate, *(h+1)) - xlate);
				*b = (char)((p1 * 16) + p2);
			}
			/*free(datastr);*/
			/*free(opstr);*/

			yywarn("Raw option %d length %d stored at %p with data at %p",
				rawop->opnum, rawop->datalen, (void*)rawop, (void*)rawop->data);

			MAKE_CFLIST(l, DHCPOPT_RAW, NULL, NULL);
			l->ptr = rawop;
			yyval.list = l;
		}
break;
case 59:
#line 745 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_DNS, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 60:
#line 753 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_DNSNAME, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 61:
#line 761 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_NTP, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 62:
#line 769 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_REFRESHTIME, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 63:
#line 777 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_NIS, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 64:
#line 785 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_NISNAME, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 65:
#line 793 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_NISP, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 66:
#line 801 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_NISPNAME, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 67:
#line 809 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_BCMCS, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 68:
#line 817 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, DHCPOPT_BCMCSNAME, NULL, NULL);
			/* currently no value */
			yyval.list = l;
		}
break;
case 69:
#line 828 "cfparse.y"
	{
			struct dhcp6_range range0, *range;

			memset(&range0, 0, sizeof(range0));
			if (inet_pton(AF_INET6, yystack.l_mark[-2].str, &range0.min) != 1) {
				yywarn("invalid IPv6 address: %s", yystack.l_mark[-2].str);
				free(yystack.l_mark[-2].str);
				free(yystack.l_mark[0].str);
				return (-1);
			}
			if (inet_pton(AF_INET6, yystack.l_mark[0].str, &range0.max) != 1) {
				yywarn("invalid IPv6 address: %s", yystack.l_mark[0].str);
				free(yystack.l_mark[-2].str);
				free(yystack.l_mark[0].str);
				return (-1);
			}
			free(yystack.l_mark[-2].str);
			free(yystack.l_mark[0].str);

			if ((range = malloc(sizeof(*range))) == NULL) {
				yywarn("can't allocate memory");
				return (-1);
			}
			*range = range0;

			yyval.range = range;
		}
break;
case 70:
#line 859 "cfparse.y"
	{
			struct dhcp6_prefix pconf0, *pconf;

			memset(&pconf0, 0, sizeof(pconf0));
			if (inet_pton(AF_INET6, yystack.l_mark[-1].str, &pconf0.addr) != 1) {
				yywarn("invalid IPv6 address: %s", yystack.l_mark[-1].str);
				free(yystack.l_mark[-1].str);
				return (-1);
			}
			free(yystack.l_mark[-1].str);
			/* validate other parameters later */
			pconf0.plen = 128; /* XXX this field is ignored */
			if (yystack.l_mark[0].num < 0)
				pconf0.pltime = DHCP6_DURATION_INFINITE;
			else
				pconf0.pltime = (uint32_t)yystack.l_mark[0].num;
			pconf0.vltime = pconf0.pltime;

			if ((pconf = malloc(sizeof(*pconf))) == NULL) {
				yywarn("can't allocate memory");
				return (-1);
			}
			*pconf = pconf0;

			yyval.prefix = pconf;
		}
break;
case 71:
#line 886 "cfparse.y"
	{
			struct dhcp6_prefix pconf0, *pconf;

			memset(&pconf0, 0, sizeof(pconf0));
			if (inet_pton(AF_INET6, yystack.l_mark[-2].str, &pconf0.addr) != 1) {
				yywarn("invalid IPv6 address: %s", yystack.l_mark[-2].str);
				free(yystack.l_mark[-2].str);
				return (-1);
			}
			free(yystack.l_mark[-2].str);
			/* validate other parameters later */
			pconf0.plen = 128; /* XXX */
			if (yystack.l_mark[-1].num < 0)
				pconf0.pltime = DHCP6_DURATION_INFINITE;
			else
				pconf0.pltime = (uint32_t)yystack.l_mark[-1].num;
			if (yystack.l_mark[0].num < 0)
				pconf0.vltime = DHCP6_DURATION_INFINITE;
			else
				pconf0.vltime = (uint32_t)yystack.l_mark[0].num;

			if ((pconf = malloc(sizeof(*pconf))) == NULL) {
				yywarn("can't allocate memory");
				return (-1);
			}
			*pconf = pconf0;

			yyval.prefix = pconf;
		}
break;
case 72:
#line 919 "cfparse.y"
	{
			struct dhcp6_prefix pconf0, *pconf;

			memset(&pconf0, 0, sizeof(pconf0));
			if (inet_pton(AF_INET6, yystack.l_mark[-3].str, &pconf0.addr) != 1) {
				yywarn("invalid IPv6 address: %s", yystack.l_mark[-3].str);
				free(yystack.l_mark[-3].str);
				return (-1);
			}
			free(yystack.l_mark[-3].str);
			/* validate other parameters later */
			pconf0.plen = yystack.l_mark[-1].num;
			if (yystack.l_mark[0].num < 0)
				pconf0.pltime = DHCP6_DURATION_INFINITE;
			else
				pconf0.pltime = (uint32_t)yystack.l_mark[0].num;
			pconf0.vltime = pconf0.pltime;

			if ((pconf = malloc(sizeof(*pconf))) == NULL) {
				yywarn("can't allocate memory");
				return (-1);
			}
			*pconf = pconf0;

			yyval.prefix = pconf;
		}
break;
case 73:
#line 946 "cfparse.y"
	{
			struct dhcp6_prefix pconf0, *pconf;

			memset(&pconf0, 0, sizeof(pconf0));
			if (inet_pton(AF_INET6, yystack.l_mark[-4].str, &pconf0.addr) != 1) {
				yywarn("invalid IPv6 address: %s", yystack.l_mark[-4].str);
				free(yystack.l_mark[-4].str);
				return (-1);
			}
			free(yystack.l_mark[-4].str);
			/* validate other parameters later */
			pconf0.plen = yystack.l_mark[-2].num;
			if (yystack.l_mark[-1].num < 0)
				pconf0.pltime = DHCP6_DURATION_INFINITE;
			else
				pconf0.pltime = (uint32_t)yystack.l_mark[-1].num;
			if (yystack.l_mark[0].num < 0)
				pconf0.vltime = DHCP6_DURATION_INFINITE;
			else
				pconf0.vltime = (uint32_t)yystack.l_mark[0].num;

			if ((pconf = malloc(sizeof(*pconf))) == NULL) {
				yywarn("can't allocate memory");
				return (-1);
			}
			*pconf = pconf0;

			yyval.prefix = pconf;
		}
break;
case 74:
#line 979 "cfparse.y"
	{
			struct dhcp6_poolspec* pool;

			if ((pool = malloc(sizeof(*pool))) == NULL) {
				yywarn("can't allocate memory");
				free(yystack.l_mark[-1].str);
				return (-1);
			}
			if ((pool->name = strdup(yystack.l_mark[-1].str)) == NULL) {
				yywarn("can't allocate memory");
				free(yystack.l_mark[-1].str);
				return (-1);
			}
			free(yystack.l_mark[-1].str);

			/* validate other parameters later */
			if (yystack.l_mark[0].num < 0)
				pool->pltime = DHCP6_DURATION_INFINITE;
			else
				pool->pltime = (uint32_t)yystack.l_mark[0].num;
			pool->vltime = pool->pltime;

			yyval.pool = pool;
		}
break;
case 75:
#line 1004 "cfparse.y"
	{
			struct dhcp6_poolspec* pool;

			if ((pool = malloc(sizeof(*pool))) == NULL) {
				yywarn("can't allocate memory");
				free(yystack.l_mark[-2].str);
				return (-1);
			}
			if ((pool->name = strdup(yystack.l_mark[-2].str)) == NULL) {
				yywarn("can't allocate memory");
				free(yystack.l_mark[-2].str);
				return (-1);
			}
			free(yystack.l_mark[-2].str);

			/* validate other parameters later */
			if (yystack.l_mark[-1].num < 0)
				pool->pltime = DHCP6_DURATION_INFINITE;
			else
				pool->pltime = (uint32_t)yystack.l_mark[-1].num;
			if (yystack.l_mark[0].num < 0)
				pool->vltime = DHCP6_DURATION_INFINITE;
			else
				pool->vltime = (uint32_t)yystack.l_mark[0].num;

			yyval.pool = pool;
		}
break;
case 76:
#line 1035 "cfparse.y"
	{
			yyval.num = -1;
		}
break;
case 77:
#line 1039 "cfparse.y"
	{
			yyval.num = yystack.l_mark[0].num;
		}
break;
case 78:
#line 1045 "cfparse.y"
	{ yyval.list = NULL; }
break;
case 79:
#line 1047 "cfparse.y"
	{
			struct cf_list *head;

			if ((head = yystack.l_mark[-1].list) == NULL) {
				yystack.l_mark[0].list->next = NULL;
				yystack.l_mark[0].list->tail = yystack.l_mark[0].list;
				head = yystack.l_mark[0].list;
			} else {
				head->tail->next = yystack.l_mark[0].list;
				head->tail = yystack.l_mark[0].list->tail;
			}

			yyval.list = head;
		}
break;
case 80:
#line 1064 "cfparse.y"
	{ yyval.list = yystack.l_mark[0].list; }
break;
case 81:
#line 1066 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, IACONF_PREFIX, yystack.l_mark[-1].prefix, NULL);

			yyval.list = l;
		}
break;
case 82:
#line 1077 "cfparse.y"
	{
		struct cf_list *ifl;

		MAKE_CFLIST(ifl, IACONF_PIF, yystack.l_mark[-4].str, yystack.l_mark[-2].list);
		yyval.list = ifl;
	}
break;
case 83:
#line 1086 "cfparse.y"
	{ yyval.list = NULL; }
break;
case 84:
#line 1088 "cfparse.y"
	{
			struct cf_list *head;

			if ((head = yystack.l_mark[-1].list) == NULL) {
				yystack.l_mark[0].list->next = NULL;
				yystack.l_mark[0].list->tail = yystack.l_mark[0].list;
				head = yystack.l_mark[0].list;
			} else {
				head->tail->next = yystack.l_mark[0].list;
				head->tail = yystack.l_mark[0].list->tail;
			}

			yyval.list = head;
		}
break;
case 85:
#line 1106 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, IFPARAM_SLA_ID, NULL, NULL);
			l->num = yystack.l_mark[-1].num;
			yyval.list = l;
		}
break;
case 86:
#line 1114 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, IFPARAM_SLA_LEN, NULL, NULL);
			l->num = yystack.l_mark[-1].num;
			yyval.list = l;
		}
break;
case 87:
#line 1124 "cfparse.y"
	{ yyval.list = NULL; }
break;
case 88:
#line 1126 "cfparse.y"
	{
			struct cf_list *head;

			if ((head = yystack.l_mark[-1].list) == NULL) {
				yystack.l_mark[0].list->next = NULL;
				yystack.l_mark[0].list->tail = yystack.l_mark[0].list;
				head = yystack.l_mark[0].list;
			} else {
				head->tail->next = yystack.l_mark[0].list;
				head->tail = yystack.l_mark[0].list->tail;
			}

			yyval.list = head;
		}
break;
case 89:
#line 1144 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, IACONF_ADDR, yystack.l_mark[-1].prefix, NULL);

			yyval.list = l;
		}
break;
case 90:
#line 1154 "cfparse.y"
	{ yyval.list = NULL; }
break;
case 91:
#line 1156 "cfparse.y"
	{
			struct cf_list *head;

			if ((head = yystack.l_mark[-1].list) == NULL) {
				yystack.l_mark[0].list->next = NULL;
				yystack.l_mark[0].list->tail = yystack.l_mark[0].list;
				head = yystack.l_mark[0].list;
			} else {
				head->tail->next = yystack.l_mark[0].list;
				head->tail = yystack.l_mark[0].list->tail;
			}

			yyval.list = head;
		}
break;
case 92:
#line 1174 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, AUTHPARAM_PROTO, NULL, NULL);
			l->num = yystack.l_mark[-1].num;
			yyval.list = l;
		}
break;
case 93:
#line 1182 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, AUTHPARAM_ALG, NULL, NULL);
			l->num = yystack.l_mark[-1].num;
			yyval.list = l;
		}
break;
case 94:
#line 1190 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, AUTHPARAM_RDM, NULL, NULL);
			l->num = yystack.l_mark[-1].num;
			yyval.list = l;
		}
break;
case 95:
#line 1198 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, AUTHPARAM_KEY, NULL, NULL);
			l->ptr = yystack.l_mark[-1].str;
			yyval.list = l;
		}
break;
case 96:
#line 1208 "cfparse.y"
	{ yyval.num = DHCP6_AUTHPROTO_DELAYED; }
break;
case 97:
#line 1209 "cfparse.y"
	{ yyval.num = DHCP6_AUTHPROTO_RECONFIG; }
break;
case 98:
#line 1213 "cfparse.y"
	{ yyval.num = DHCP6_AUTHALG_HMACMD5; }
break;
case 99:
#line 1217 "cfparse.y"
	{ yyval.num = DHCP6_AUTHRDM_MONOCOUNTER; }
break;
case 100:
#line 1221 "cfparse.y"
	{ yyval.list = NULL; }
break;
case 101:
#line 1223 "cfparse.y"
	{
			struct cf_list *head;

			if ((head = yystack.l_mark[-1].list) == NULL) {
				yystack.l_mark[0].list->next = NULL;
				yystack.l_mark[0].list->tail = yystack.l_mark[0].list;
				head = yystack.l_mark[0].list;
			} else {
				head->tail->next = yystack.l_mark[0].list;
				head->tail = yystack.l_mark[0].list->tail;
			}

			yyval.list = head;
		}
break;
case 102:
#line 1241 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, KEYPARAM_REALM, NULL, NULL);
			l->ptr = yystack.l_mark[-1].str;
			yyval.list = l;
		}
break;
case 103:
#line 1249 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, KEYPARAM_KEYID, NULL, NULL);
			l->num = yystack.l_mark[-1].num;
			yyval.list = l;
		}
break;
case 104:
#line 1257 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, KEYPARAM_SECRET, NULL, NULL);
			l->ptr = yystack.l_mark[-1].str;
			yyval.list = l;
		}
break;
case 105:
#line 1265 "cfparse.y"
	{
			struct cf_list *l;

			MAKE_CFLIST(l, KEYPARAM_EXPIRE, NULL, NULL);
			l->ptr = yystack.l_mark[-1].str;
			yyval.list = l;
		}
break;
#line 2099 "y.tab.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            yychar = YYLEX;
            if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if (((yyn = yygindex[yym]) != 0) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
