#ifndef ARG_H
#define ARG_H

#include <stdlib.h> // size_t

#define ARG_MAX_ARGS 30 // maximum number of arguments per command
#define ARG_MAX_DESCR_LEN 4096
#define ARG_MAX_SUBC 10

// TODO: rename whole lib to "opt"
// Argument types
#define ARG_REQUIRED (1U << 1)
#define ARG_POSITIONAL (1U << 2)
#define ARG_HAS_ARG (1U << 3)
// TODO: implement these...
#define ARG_POS_MULTIPLE (1U << 4)
#define ARG_POS_REQUIRED (1U << 5)

#define ARG(s, l, flag, help) (&(arg_arg_t){s, l, flag, help})
#define POSITIONAL_ARG(name, flag, help)                                       \
    (&(arg_arg_t){'\0', name, ARG_POSITIONAL, help})

typedef struct arg_arg {
    char shortname;
    char *longname;
    unsigned int type;
    char *help;
} arg_arg_t;

typedef struct arg_config {
    unsigned int parse_long;
    unsigned int add_help;
    unsigned int subcommand_required;
    const char description[ARG_MAX_DESCR_LEN];
    const char *subcommands[ARG_MAX_SUBC];
    arg_arg_t *args[ARG_MAX_ARGS];
} arg_config_t;

typedef struct arg {
    // external API
    int optopt;
    int optind;
    int optpos;
    char *optarg;

    // internal API
    //   arguments
    size_t argind; // Index of parsed argument
    size_t n_args; // Number of arguments provided (not inc pos)
    //   subcommands
    size_t n_subc;         // Number of subcommands configured
    size_t subc_idx;       // Current index of found subarg
    unsigned int has_subc; // Are we configured to use subc?
    //   positional arguments
    size_t n_pos_seen;             // Number of positional args encountered
    size_t n_arg_pos;              // Num positional args
    int arg_pos_idx[ARG_MAX_ARGS]; // Positional arg idx
} arg_t;

enum arg_err {
    ARG_ERR_CONFIG_TOO_MANY_ARGS = -100,
    ARG_ERR_CANNOT_ADD_HELP,
    ARG_ERR_TOO_MANY_SUBCOMMANDS,
    ARG_ERR_MISSING_ARGUMENT,
    ARG_ERR_MISSING_LONG_ARGUMENT,
    ARG_ERR_UNKNOWN_ARGUMENT,
    ARG_ERR_UNKNOWN_LONG_ARGUMENT,
    ARG_ERR_TOO_MANY_POSITIONALS,
    ARG_ERR_GENERIC,
    ARG_ERR_NO_LONG_ARGS,
    ARG_ERR_SUBC_REQUIRED,
};

enum arg_ret {
    ARG_DONE,
    ARG_OK,
    ARG_SUBC,
};

void arg_print_error(int ret, arg_t *s, char **argv);
int arg_init(arg_t *state, arg_config_t *config, unsigned int in_subc);
int arg_parse(int argc, char **argv, arg_t *s, arg_config_t *config);
void arg_usage(char *argv0, arg_t *state, char *subc_name,
               arg_config_t *config);
#endif // ARG_H
