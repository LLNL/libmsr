#
# A macro to detect the libmsr version
#
# Sets have_msr2 and have_msr1

AU_ALIAS([DE_LIBMSR_VERSION])
AC_DEFUN([DE_LIBMSR_VERSION],[
have_msr2="no"
have_msr1="no"
AC_CHECK_HEADERS([memhdlr.h],[have_msr2="yes"],[have_msr2="no"])
if test x$have_msr2 == x"no"
then
    AC_CHECK_HEADERS([msr_rapl.h],[have_msr1="yes"
AC_DEFINE([HAVE_LIBMSR1],1,[libmsr1 is available])
],[have_msr1="no"])
else
    AC_DEFINE([HAVE_LIBMSR2],1,[libmsr2 is available])
fi
AM_CONDITIONAL([HAVE_LIBMSR1],[test x$have_msr1 = xyes])
AM_CONDITIONAL([HAVE_LIBMSR2],[test x$have_msr2 = xyes])
])
