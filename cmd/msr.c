#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#define CMD_PREFIX "msr-"

///
/// Print some simple usage information.
///
void usage(const char *cmd) {
  printf("Usage: %s [-h] <command> [<args>]\n", cmd);
  printf("\n");
  printf("Options:\n");
  printf("   -h           show this message.\n");
  printf("\n");
  printf("Available commands are:\n");
  printf("   turbo        enable/disable turbo.\n");
  exit(1);
}


///
/// Looks for a subcommand in the same directory as a command.
///
/// Example:
///    subcommand_path("/usr/bin/msr", "msr-", "turbo")
///
/// Would return:
///    /usr/bin/msr-turbo
///
const char *subcommand_path(const char *command,
                            const char *prefix,

                            const char *subcommand) {
  char *command_path = realpath(command, NULL);
  const char *command_dir = dirname(command_path);

  size_t pathlen = strlen(command_dir) + strlen(prefix) + strlen(subcommand) + 2;
  char *subcommand_path = malloc(sizeof(char) * pathlen);
  sprintf(subcommand_path, "%s/%s%s", command_dir, prefix, subcommand);

  free(command_path);
  return subcommand_path;
}


///
/// Main msr command.  Searches for a subcommand in the bin directory with the
/// executable, and exec's it.
///
int main(int argc, char **argv) {
  const char *cmd = argv[0];
  if (argc < 2) {
    usage(cmd);
  }

  // find the first non-option arg.
  int arg_start;
  for (arg_start=1; arg_start < argc; arg_start++) {
    const char *arg = argv[arg_start];
    if (!strlen(arg)) continue;
    if (arg[0] == '-') usage(cmd);  // -h prints usage, any other arg is error
    break;
  }

  // figure out what subcommand we're calling.
  const char *subcommand = argv[1];
  const char *path = subcommand_path(argv[0], CMD_PREFIX, subcommand);

  // exec the subcommand with the rest of the args.
  if (execv(path, &argv[arg_start])) {
    if (errno == ENOENT) {
      fprintf(stderr, "No such command: %s\n", subcommand);
      usage(cmd);
    } else {
      fprintf(stderr, "%s.\n", strerror(errno));
    }
    exit(1);
  }
  exit(0);
}
