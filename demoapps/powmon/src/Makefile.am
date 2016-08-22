###############################################################################
AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_FLAGS = -I m4

LDFLAGS += -lmsr
if HAVE_LIBMSR1
RAPL_SRC = rapl1.c
endif
if HAVE_LIBMSR2
RAPL_SRC = rapl2.c
endif

bin_PROGRAMS = powmon power_wrapper_static power_wrapper_dynamic

powmon_SOURCES = highlander.c powmon.c $(RAPL_SRC)
power_wrapper_static_SOURCES = highlander.c power_wrapper_static.c $(RAPL_SRC)
power_wrapper_dynamic_SOURCES = highlander.c power_wrapper_dynamic.c $(RAPL_SRC)
