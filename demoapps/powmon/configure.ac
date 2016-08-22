m4_define([powmon_version],[1.1.b])

###############################################################################
# Auto configure initialization
AC_INIT(powmon, [powmon_version], [dellswor@cs.uoregon.edu])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])

###############################################################################
# Identify the libmsr to build against
DE_LIBMSR_VERSION("")

# Check to for extra flags for clock support
AC_SEARCH_LIBS([clock_gettime],[rt])

###############################################################################
# Generate files
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
