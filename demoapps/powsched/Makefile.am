##########################################################################################
AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_FLAGS = -I m4

if HAVE_LIBMSR1
RAPL_SRC = rapl1.c
endif
if HAVE_LIBMSR2
RAPL_SRC = rapl2.c
endif

bin_PROGRAMS = power_monitor power_actuator powsched

LDFLAGS += -L../lib/.libs -lexpose -lexposeutil

powsched_SOURCES = powsched.c
power_monitor_SOURCES = power_monitor.c
power_actuator_SOURCES = power_actuator.c $(RAPL_SRC)
