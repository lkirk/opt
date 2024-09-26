#ifndef OPT_H
#define OPT_H

#include <stdlib.h> // size_t

#define OPT_MAX_ARGS 30 // maximum number of arguments per command
#define OPT_MAX_DESCR_LEN 4096
#define OPT_MAX_SUBC 10
#define OPT_MAX_NESTED_SUBC 10

// TODO: rename whole lib to "opt"
// Argument types
// TODO: validate that all required arguments were provided?
#define OPT_REQUIRED (1U << 1)
#define OPT_POSITIONAL (1U << 2)
#define OPT_HAS_ARG (1U << 3)
// TODO: implement these...
#define OPT_POSITIONAL_MULTIPLE (1U << 4)
#define OPT_POSITIONAL_REQUIRED (1U << 5)

#define ARG(s, l, flag, help) (&(opt_arg_t){s, l, flag, help})
#define POSITIONAL_ARG(name, flag, help)                                       \
    (&(opt_arg_t){'\0', name, OPT_POSITIONAL | (flag), help})

#ifdef OPT_TEST
#define OPT_PRIVATE
#else
#define OPT_PRIVATE static
#endif
#define OPT_UNUSED(x) x __attribute__((__unused__))

typedef struct opt_arg {
    char shortname;
    char *longname;
    unsigned int type;
    char *help;
} opt_arg_t;

typedef struct opt_config {
    unsigned int parse_long;
    unsigned int add_help;
    unsigned int subcommand_required;
    const char description[OPT_MAX_DESCR_LEN];
    const char *subcommands[OPT_MAX_SUBC];
    opt_arg_t *args[OPT_MAX_ARGS];
} opt_config_t;

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
    char *subc_name[OPT_MAX_NESTED_SUBC]; // Current subc name(s)
    size_t n_subc;                        // Number of subcommands configured
    size_t subc_idx;                      // Current index of found subcommand
    unsigned int has_subc; // Are we configured to use subcommands?
    //   positional arguments
    size_t n_pos;              // Num positional args
    size_t n_pos_seen;         // Number of positional args encountered
    unsigned int has_mult_pos; // Do we expect multiple pos args?
    int pos_idx[OPT_MAX_ARGS]; // Positional arg idx
} opt_t;

enum opt_err {
    OPT_ERR_CONFIG_TOO_MANY_ARGS = -100,
    OPT_ERR_CANNOT_ADD_HELP,
    OPT_ERR_TOO_MANY_SUBCOMMANDS,
    OPT_ERR_MISSING_ARGUMENT,
    OPT_ERR_MISSING_LONG_ARGUMENT,
    OPT_ERR_UNKNOWN_ARGUMENT,
    OPT_ERR_UNKNOWN_LONG_ARGUMENT,
    OPT_ERR_TOO_MANY_POSITIONALS,
    OPT_ERR_GENERIC,
    OPT_ERR_NO_LONG_ARGS,
    OPT_ERR_SUBC_REQUIRED,
    OPT_ERR_UNKNOWN_SUBC,
    OPT_ERR_INIT,
};

enum opt_ret {
    OPT_DONE,
    OPT_OK,
    OPT_SUBC,
};

// Public API
void opt_print_error(int ret, opt_t *s, char **argv);
int opt_init(opt_t *state, opt_config_t *config, unsigned int in_subc);
int opt_parse(int argc, char **argv, opt_t *s, const opt_config_t *config);
void opt_usage(char *argv0, opt_t *state, char *subc_name,
               opt_config_t *config);

#ifdef OPT_TEST
OPT_PRIVATE int parse_short(opt_t *s, size_t n_args, opt_arg_t *const *args,
                            int argc, char **argv);
OPT_PRIVATE int parse_long(opt_t *s, size_t n_args, opt_arg_t *const *args,
                           int argc, char **argv);
OPT_PRIVATE int
parse_subcommand_or_positional(opt_t *s, const char *const *subcs, char **argv);
#endif

#endif // OPT_H
