##########################################################################################
# Auto configure initialization
AC_INIT(powsched, powsched-0.0)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])

##########################################################################################
# MPI configuration
# If we're going to use MPI we must set it early
use_mpi=no
have_mpi=no
AC_ARG_ENABLE([mpi],
    [AS_HELP_STRING( [--enable_mpi], [use mpi (default=yes)]
) ],
    [use_mpi=$enableval], [use_mpi=yes]
)

if test "x$use_mpi" == x"yes"
then
AX_PROG_CC_MPI([test x"yes" != xno],[have_mpi=yes],[have_mpi=no
      AC_MSG_FAILURE([MPI compiler requested but not found.])
  ])
fi

# Add C99 flags
AC_PROG_CC_C99

# Identify host platform
AC_CANONICAL_HOST

##########################################################################################
# libMSR configuration
use_msr=yes
have_msr=no
AC_ARG_ENABLE([msr],
    [AS_HELP_STRING( [--disable-msr], [use libmsr (default=yes)]
) ],
    [use_msr=no], [use_msr=yes]
)

##########################################################################################
# Tests

# Check for pthreads
AX_PTHREAD([have_pthreads="yes"],[have_pthreads="no"])
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"

# Check for the math library
AC_SEARCH_LIBS([floor],[m],[have_m="yes"],[have_m="no"])

# Check for realtime clock support
AC_SEARCH_LIBS([clock_getres],[rt],[have_rt="yes"],[have_rt="no"])

# Check for libexpose
AC_CHECK_HEADERS([expose.h exposeutil.h])
AC_SEARCH_LIBS([EXPOSE_init], [expose], [have_expose="yes"], [have_expose="no"])

# Check for libMSR
if test x"$use_msr" == x"yes"
then
DE_LIBMSR_VERSION("")
if test x"$have_msr1" == x"yes" 
then
have_msr="yes"
fi

if test x"$have_msr2" == x"yes" 
then
have_msr="yes"
fi

fi


##########################################################################################
# End of configure summary output
DE_STAT("Build with MPI", $have_mpi)
DE_STAT("pthreads found", $have_pthreads)
DE_STAT("libmath found", $have_m)
DE_STAT("librt found", $have_rt)
DE_STAT("libmsr found", $have_msr)
DE_STAT("libexpose found", $have_expose)

##########################################################################################
# Generate files
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

