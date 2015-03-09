#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>

// prefixes with which to search for subcommands, relative to msr command.
static const char *cmd_search_paths[] = {
  "./msr-",                   // alongside the msr executable.
  "../libexec/libmsr/msr-",   // in libexec/libmsr in the install prefix.
  NULL
};


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
/// Looks for a subcommand relative to the command location.
///
const char *subcommand_path(const char *command,
                            const char **rel_paths,
                            const char *subcommand) {
  char command_path[PATH_MAX];
  realpath(command, command_path);
  const char *command_dir = dirname(command_path);

  const char ** relpath;
  for (relpath = rel_paths; *relpath; relpath++) {
    char subcommand_path[PATH_MAX];
    snprintf(subcommand_path, PATH_MAX, "%s/%s%s", command_dir, *relpath, subcommand);

    struct stat st;
    int err = stat(subcommand_path, &st);
    if (err != -1 && !S_ISDIR(st.st_mode) && (st.st_mode & S_IXUSR)) {
      return realpath(subcommand_path, NULL);
    }
  }
  return NULL;
}


///
/// Main msr command.  Searches for a subcommand in the bin directory with the
/// executable, and exec's it.
///
int main(int argc, char **argv) {
  if (argc < 2) {
    usage(argv[0]);
  }

  // Figure out where the exe lives.
  char cmd[PATH_MAX];
  readlink("/proc/self/exe", cmd, PATH_MAX);

  // find the first non-option arg.
  int arg_start;
  for (arg_start=1; arg_start < argc; arg_start++) {
    const char *arg = argv[arg_start];
    if (!strlen(arg)) continue;
    if (arg[0] == '-') usage(argv[0]);  // -h prints usage, any other arg is error
    break;
  }

  // figure out what subcommand we're calling.
  const char *subcommand = argv[1];
  const char *path = subcommand_path(cmd, cmd_search_paths, subcommand);

  // no command found -- bail.
  if (!path) {
    fprintf(stderr, "No such command: %s\n", subcommand);
    usage(argv[0]);
  }

  // exec the subcommand with the rest of the args.
  if (execv(path, &argv[arg_start])) {
    fprintf(stderr, "%s.\n", strerror(errno));
    exit(1);
  }

  exit(0);
}
