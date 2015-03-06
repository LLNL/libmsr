#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "msr_turbo.h"

void usage() {
  printf("Usage: msr turbo [-h] <enable|disable>\n");
  printf("\n");
  printf("Options:\n");
  printf("   -h           print this message.\n");
  printf("\n");
  printf("Subcommands:\n");
  printf("   enable       enable turbo.\n");
  printf("   disable      disable turbo.\n");
  printf("   dump         print turbo status.\n");

  exit(1);
}


int main(int argc, char **argv) {
  if (argc < 2) {
    usage();
  }

  // find the first non-option arg.
  int arg_start;
  for (arg_start=1; arg_start < argc; arg_start++) {
    const char *arg = argv[arg_start];
    if (!strlen(arg)) continue;
    if (arg[0] == '-') usage();  // -h prints usage, any other arg is error
    break;
  }

  // TODO: enable and disable are void -- should they return errors?
  const char *subcommand = argv[arg_start];
  if (strcmp(subcommand, "enable") == 0) {
    enable_turbo();
  } else if (strcmp(subcommand, "disable") == 0) {
    disable_turbo();
  } else if (strcmp(subcommand, "dump") == 0) {
    dump_turbo();
  } else {
    usage();
  }

  exit(0);
}
