dnl Process this file with autoconf to produce a configure script.
AC_INIT([wide-dhcpv6],[20080615])

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_LEX

dnl Checks for libraries.
dnl AC_CHECK_LIB(c, kvm_open, [], [AC_CHECK_LIB(kvm, kvm_open)])
dnl AC_CHECK_LIB(mld, nlist)

dnl pcap library
dnl CFLAGS="-I. $CFLAGS"
dnl AC_MSG_CHECKING(for pcap library/header)
dnl for dir in /usr/local/libpcap /usr/local/v6 /usr/local \
dnl 		../libpcap* ../../libpcap*; do
dnl 	ac_cv_pcap_lib=no
dnl 	ac_cv_pcap_include=no
dnl 	if test -d $dir -a -f $dir/libpcap.a; then
dnl 		ac_cv_pcap_lib="$dir"
dnl 	elif test -d $dir/lib -a -f $dir/lib/libpcap.a; then
dnl 		ac_cv_pcap_lib="$dir/lib"
dnl 	fi
dnl 	if test -d $dir -a -f $dir/pcap.h; then
dnl 		ac_cv_pcap_include="$dir"
dnl 	elif test -d $dir/include -a -f $dir/include/pcap.h; then
dnl 		ac_cv_pcap_include="$dir/include"
dnl 	fi
dnl 	if test "$ac_cv_pcap_lib" != "no" -a "$ac_cv_pcap_include" != "no"; then
dnl 		LIBS="-L$ac_cv_pcap_lib $LIBS"
dnl 		CFLAGS="-I$ac_cv_pcap_include $CFLAGS"
dnl 		break
dnl 	fi
dnl done
dnl if test "$ac_cv_pcap_lib" = "no" -a "$ac_cv_pcap_include" = "no"; then
dnl 	AC_MSG_RESULT(no)
dnl else
dnl 	AC_MSG_RESULT($ac_cv_pcap_lib and $ac_cv_pcap_include)
dnl fi
dnl AC_CHECK_LIB(pcap, pcap_lookupdev, [],
dnl 	[echo "Fatal: libpcap.a not found"
dnl 	exit 1])
dnl AC_TRY_COMPILE([#include <pcap.h>], [], [], [dnl
dnl 	echo "Fatal: pcap.h not found"
dnl 	exit 1])

CFLAGS="$CFLAGS -I\$(srcdir)"

dnl On Sun systems, we need to use the standards-compliant 3XNET functions
if test -x /usr/bin/sun && /usr/bin/sun; then
	CFLAGS="$CFLAGS -D_XOPEN_SOURCE=500 -D__EXTENSIONS__"
	LDFLAGS="-lxnet -ldevinfo -lnsl -lsocket -lrt"
fi

AC_MSG_CHECKING(for sys/queue.h)
AC_TRY_COMPILE([#include <sys/queue.h>],
	[TAILQ_HEAD(test, none);],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)
	CFLAGS="$CFLAGS -I\$(srcdir)/missing"])

AC_EGREP_CPP(yes,
[#include <netinet/in.h>
#ifdef __KAME__
 yes
#endif], [result=kame], [result=regular])
AC_MSG_CHECKING(for getaddrinfo/getnameinfo library)
AC_MSG_RESULT($result)
case $result in
kame)	AC_DEFINE(INET6)
	o_LIBS="$LIBS"
	LIBS="$LIBS -L/usr/local/v6/lib"
	AC_CHECK_LIB(inet6, getaddrinfo, [], [LIBS="$o_LIBS"])
	;;
*)	;;
esac

AC_REPLACE_FUNCS(getaddrinfo)
AC_REPLACE_FUNCS(getnameinfo)
AC_REPLACE_FUNCS(getifaddrs)
AC_CHECK_FUNCS(if_nametoindex)
AC_REPLACE_FUNCS(strlcpy strlcat)
AC_REPLACE_FUNCS(daemon)
AC_REPLACE_FUNCS(warnx)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/time.h syslog.h unistd.h ifaddrs.h)
AC_MSG_CHECKING(for socklen_t)
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],
	[socklen_t x;  x = 1; return x;],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)
	AC_CHECK_TYPE(socklen_t, int)])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE
AC_CHECK_TYPES([sig_atomic_t], , [AC_DEFINE(sig_atomic_t, u_long)], [#include <sys/types.h>
	#include <signal.h>])


dnl Checks for library functions.
AC_FUNC_GETPGRP
AC_PROG_GCC_TRADITIONAL
AC_FUNC_SETPGRP
AC_TYPE_SIGNAL
AC_CHECK_FUNCS(mktime select socket)
AC_CHECK_FUNCS(clock_gettime)

dnl configure local DB directory
AC_MSG_CHECKING(for local DB directory)
AC_ARG_WITH(localdbdir,
[  --with-localdbdir=VALUE where to put local DB files],
    localdbdir="$withval", localdbdir="/var/db")
AC_MSG_RESULT($localdbdir)
AC_SUBST(localdbdir)

AC_MSG_CHECKING(if --enable-pedant option is specified)
AC_ARG_ENABLE(pedant, [  --enable-pedant         pedantic compilation],
	[if test "$enableval" = "yes"; then
		CFLAGS="-Wall -Werror $CFLAGS"
	fi], [enableval=no])
AC_MSG_RESULT($enableval);

dnl Checks predeclared identifers of function names
AC_MSG_CHECKING(ANSI predeclared __func__)
AC_TRY_COMPILE([],
[void func(void) {printf("%s\n", __func__);}],
	[AC_MSG_RESULT(yes)
	 pred_func_id=ansi],
	[AC_MSG_RESULT(no)
	 pred_func_id=no])
if test $pred_func_id = no; then
AC_MSG_CHECKING(gcc predelcared __FUNCTION__)
	AC_TRY_COMPILE([],
	[void func(void) {printf("%s\n", __FUNCTION__);}],
		[AC_MSG_RESULT(yes)
		 pred_func_id=gcc],
		[AC_MSG_RESULT(no)
		 pred_func_id=no])
fi
case $pred_func_id in
	ansi)
		AC_DEFINE(HAVE_ANSI_FUNC)
		;;
	gcc)
		AC_DEFINE(HAVE_GCC_FUNCTION)
		;;
esac

dnl DHCP option type values not officially defined
dnl (no such option now)

dnl Checks the existence of TAILQ_FOREACH_REVERSE
AC_MSG_CHECKING(checking the existence TAILQ_FOREACH_REVERSE)
AC_EGREP_CPP(yes,
[#include <sys/queue.h>
 #ifdef TAILQ_FOREACH_REVERSE
  yes
#endif], [result=yes], [result=no])
AC_MSG_RESULT($result)
case $result in
yes)	tailq_foreach=yes
 	;;
*)	tailq_foreach=no
	;;
esac

dnl Checks the argument order of TAILQ_FOREACH_REVERSE if it exists
if test $tailq_foreach = yes ; then
	AC_MSG_CHECKING(argument order of TAILQ_FOREACH_REVERSE)
	AC_TRY_COMPILE([],
	[
	#include <stdio.h>
	#include <sys/queue.h>

	TAILQ_HEAD(tailhead, entry) head = TAILQ_HEAD_INITIALIZER(head);
	struct entry {
		TAILQ_ENTRY(entry) entries;
	} *p;

	int i;
	struct tailhead *headp;

	TAILQ_FOREACH_REVERSE(p, headp, tailhead, entries) {
	}
	],
	[AC_MSG_RESULT(new)
	 AC_DEFINE(HAVE_TAILQ_FOREACH_REVERSE)],
	[AC_MSG_RESULT(old)
	 AC_DEFINE(HAVE_TAILQ_FOREACH_REVERSE_OLD)]);
fi

dnl Checks for other features
AC_MSG_CHECKING(if you have a length field in struct sockaddr*)
AC_CACHE_VAL(ts_cv_sa_len, [dnl
 AC_TRY_COMPILE([
 #include <sys/types.h>
 #include <sys/socket.h>
 ], [ 
 struct sockaddr a;
 a.sa_len = 1;
 ], [ts_cv_sa_len="yes"], [ts_cv_sa_len="no"])])
AC_MSG_RESULT($ts_cv_sa_len);
if test "$ts_cv_sa_len" = yes; then
	AC_DEFINE([HAVE_SA_LEN], 1, [Define to 1 if you have a length field in struct sockaddr])
fi

dnl configure the ownership of the programs
AC_MSG_CHECKING(the owner of the programs)
AC_ARG_WITH(user,
[  --with-user=USER	set the owner of the programs],
    user="$withval", user="bin")
AC_MSG_RESULT($user)
AC_SUBST(user)

dnl configure the group ownership of the programs
AC_MSG_CHECKING(the group owner of the programs)
AC_ARG_WITH(group,
[  --with-group=GROUP	set the group owner of the programs],
    group="$withval", group="bin")
AC_MSG_RESULT($group)
AC_SUBST(group)

AC_CHECK_HEADERS(stdarg.h)

AC_OUTPUT(Makefile)
