#define _POSIX_C_SOURCE 200809

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SUBC 512

typedef enum subc {
  SUBC_1,
  SUBC_2,
  N_SUBC,
} subc_t;

const char *subc_name[3] = {"command1", "command2", NULL};

void usage(char *progname) {
  fprintf(stderr, "usage: %s <subcommand> [args]\n", progname);
  fprintf(stderr, "       See %s -h for more detail\n", progname);
  exit(EXIT_FAILURE);
}

void die_usage(char *progname, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  usage(progname);
}

int match_subc(char *cmd) {
  char *null = memchr(cmd, '\0', MAX_SUBC);
  if (!null)
    return -1; // exceeded length or not null terminated
  for (subc_t sc = 0; sc < N_SUBC; sc++)
    if (!strcmp(cmd, subc_name[sc]))
      return sc;
  return -1;
}

int main(int argc, char **argv) {
  int flag;
  char *t_arg;
  int o_flag = 0;
  subc_t subcommand = 0;

  while (optind < argc) {
    if ((flag = getopt(argc, argv, "t:oh")) != 1) {
      switch (flag) {
      case 't':
        t_arg = optarg;
        break;
      case 'o':
        o_flag = 1;
        break;
      case 'h':
        usage(argv[0]);
        break;
      case '?':
        die_usage(argv[0], "Error: Unrecognized option: %c", optopt);
        break;
      }
    }
  }
  fprintf(stdout, "t_arg=%s\no_flag=%d\nsubcommand=%s\n", t_arg, o_flag,
          subc_name[subcommand]);
}
