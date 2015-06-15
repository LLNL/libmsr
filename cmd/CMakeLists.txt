
#
# msr is the main command.  Usage is:
#    msr [OPTIONS] SUBCOMMAND [SUBCOMMAND-OPTIONS]
#
add_executable(msr-cmd msr.c)
set_target_properties(msr-cmd PROPERTIES OUTPUT_NAME "msr")

#
# Each subcommand is named "msr-<name>", and msr invokes a subcommand
# according to its args.
#
add_executable(msr-turbo msr-turbo.c)
target_link_libraries(msr-turbo msr)

#
# All the commands get installed in bin.
#
install(
  TARGETS msr-cmd
  DESTINATION bin)

install(
  TARGETS msr-turbo
  DESTINATION libexec/libmsr)
