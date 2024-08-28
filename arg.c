#include <stdio.h> // for usage
#include <string.h>

#include "arg.h"

// Limits
#define MAX_SUBC_LEN 256
#define MAX_ARG_LEN 512

arg_arg_t *ARG_DEFAULT_HELP_FLAG =
    ARG('h', "help", 0, "Print help message and exit.");

static unsigned int is_doubledash(char *s) {
    return s[0] != '\0' && s[0] == '-' && s[1] == '\0' && s[1] == '-';
}

static unsigned int is_short(char *s) {
    return s[0] != '\0' && s[0] == '-' && s[1] != '-';
}

static unsigned int is_long(char *s) {
    return s[0] != '\0' && s[0] == '-' && s[1] != '\0' && s[1] == '-' &&
           s[2] != '\0';
}

unsigned int is_subcommand(char *s, char **subcs) {
    int i;
    for (i = 0; subcs[i] && i < ARG_MAX_SUBC; i++)
        if (!strncmp(s, subcs[i], MAX_SUBC_LEN))
            return 1;
    return 0;
}

// TODO: all max checks... should we add +1 to the max?
int arg_init(arg_t *state, arg_config_t *config, unsigned int in_subc) {
    // validate config and initialize parsing state
    int i;
    arg_arg_t **args = config->args;

    if (!in_subc) {
        memset(state, 0, sizeof(*state));
        state->optind = 1;
        state->optpos = 1;
    }
    // Null indices are -1
    memset(state->arg_pos_idx, -1, sizeof(*state->arg_pos_idx) * ARG_MAX_ARGS);

    for (i = 0; args[i] && i < ARG_MAX_ARGS; i++)
        if (args[i]->type & ARG_POSITIONAL)
            state->arg_pos_idx[state->n_arg_pos++] = i;
    state->n_args = i;
    if (i == ARG_MAX_ARGS)
        return -1; // too many args or no null terminator
    if (config->add_help) {
        if (i + 1 == (ARG_MAX_ARGS))
            return -1; // cannot add help, would exceed max
        config->args[i] = ARG_DEFAULT_HELP_FLAG;
        state->n_args++;
    }

    for (i = 0; config->subcommands[i] && i < ARG_MAX_SUBC; i++)
        ;
    if (i == ARG_MAX_SUBC)
        return -1; // too many subc or no null terminator
    state->has_subc = i > 0 ? 1 : 0;
    state->n_subc = i;

    return 0;
}

// TODO: arg string msg.

int parse_short(arg_t *s, int n_args, arg_arg_t **args, int argc, char **argv) {
    int i;
    arg_arg_t *arg = NULL;
    char *arg_str;

    arg_str = argv[s->optind];
    for (i = 0; i < n_args; i++)
        if (args[i]->shortname == arg_str[s->optpos]) {
            arg = args[i];
            break;
        }
    if (i == n_args)
        return ARG_ERR_UNKNOWN_ARGUMENT; // arg unknown... err str... keep
                                         // parsing?
    s->argind = i;
    arg = args[i];
    if (arg->type & ARG_REQUIRED) {
        s->optpos++;
        if (!arg_str[s->optpos] && s->optind == argc - 1)
            return ARG_ERR_MISSING_ARGUMENT;
        if (!arg_str[s->optpos])
            s->optarg = argv[++s->optind]; // TODO: validate this is not a flag?
        else if (arg_str[s->optpos] == '=')
            s->optarg = &arg_str[++s->optpos];
        else
            s->optarg = &arg_str[s->optpos];
        s->optpos = 1;
        s->optind++;
        return ARG_OK;
    }
    if (!arg_str[++s->optpos]) {
        s->optind++;
        s->optpos = 1;
    }
    return ARG_OK;
}

int parse_long(arg_t *s, int n_args, arg_arg_t **args, int argc, char **argv) {
    int i;
    size_t arg_len;
    arg_arg_t *arg = NULL;
    char *arg_str;

    arg_str = argv[s->optind];
    s->optpos = 2;
    for (i = 0; i < n_args; i++) {
        arg_len = strlen(args[i]->longname);
        if (!strncmp(args[i]->longname, &arg_str[s->optpos], arg_len)) {
            arg = args[i];
            break;
        }
    }
    if (!arg)
        return ARG_ERR_UNKNOWN_LONG_ARGUMENT; // Unexpected argument
    s->argind = i;
    if (arg->type & ARG_REQUIRED) {
        s->optpos += arg_len; // advance to end of arg
        if (!arg_str[s->optpos] && s->optind == argc - 1)
            return ARG_ERR_MISSING_LONG_ARGUMENT;
        if (!arg_str[s->optpos])
            s->optarg = argv[++s->optind]; // TODO: validate this is not a flag?
        else if (arg_str[s->optpos] == '=')
            s->optarg = &arg_str[++s->optpos];
        else
            s->optarg = &arg_str[s->optpos];
        s->optpos = 1;
        s->optind++;
        return ARG_OK;
    }
    s->optind++;
    s->optpos = 1;
    return ARG_OK;
}

int parse_subcommand_or_positional(arg_t *s, const char **subcommands,
                                   size_t *n_pos_seen, size_t *n_pos,
                                   char **argv) {
    int i;
    char *arg_str;

    arg_str = argv[s->optind];
    for (i = 0; i < (int)s->n_subc; i++)
        if (!strncmp(arg_str, subcommands[i], strlen(arg_str)))
            break;
    if (i == (int)s->n_subc) { // not matched, must be positional
        if (*n_pos_seen == *n_pos) {
            return ARG_ERR_TOO_MANY_POSITIONALS; // unrecognized argument, we've
                                                 // seen all expected pos args
        } else {
            (*n_pos_seen)++;
            s->optarg = arg_str;
            s->optind++;
            return ARG_OK;
        }
    } else {
        s->subc_idx = i;
        s->optarg = arg_str;
        s->optind++;
        return ARG_SUBC;
    }
    // TODO: if no positional arguments are required, then validate the
    // subcommand
}

int arg_parse(int argc, char **argv, arg_t *s, arg_config_t *config) {
    arg_arg_t **args = config->args;
    int n_args = s->n_args;
    size_t *n_pos_seen = &s->n_pos_seen;
    size_t *n_pos = &s->n_arg_pos;

    if (s->optind == argc)
        //     return validate_end_state(s, config);
        return ARG_DONE;

    if (!argv[s->optind]) {
        return ARG_ERR_GENERIC; // not sure what the error would be here

    } else if (is_doubledash(argv[s->optind])) {
        // TODO: look in more detail into what happens after double dash
        s->optind++;
        return ARG_ERR_GENERIC; // prob not an error. but what is it? pos arg?
    } else if (is_short(argv[s->optind])) {
        return parse_short(s, n_args, args, argc, argv);
    } else if (is_long(argv[s->optind])) { // Long argument
        if (!config->parse_long)
            return ARG_ERR_NO_LONG_ARGS; // Didn't expect long args
        return parse_long(s, n_args, args, argc, argv);
    } else { // must be a positional arg or subcommand
        return parse_subcommand_or_positional(s, config->subcommands,
                                              n_pos_seen, n_pos, argv);
    }

    return ARG_ERR_GENERIC; // unexpected parsing state.. got unexpected arg?
}

void arg_usage(char *argv0, arg_t *state, char *subc_name,
               arg_config_t *config) {
    size_t i, j, len = 0, pos_arg = 0, maxlen = 0;
    arg_arg_t *arg, **args = config->args;

    // TODO: basename of argv0??
    printf("Usage: %s", argv0);
    if (subc_name)
        printf(" %s", subc_name);
    if (state->n_args > 0)
        printf(" [OPTIONS]");
    if (state->n_subc > 0) {
        if (config->subcommand_required)
            printf(" COMMAND");
        else
            printf(" [COMMAND]");
    }
    printf("\n\n%s\n", config->description);
    if (state->n_subc > 0)
        printf("\nSubcommands:\n");
    for (i = 0; i < state->n_subc; i++)
        printf("  %s\n", config->subcommands[i]); // TODO: subcommand help
    if (state->n_args > 0) {
        if (state->n_subc > 0)
            printf("\nGlobal Options:\n");
        else
            printf("\nOptions:\n");
    }

    if (config->parse_long)
        for (i = 0; i < state->n_args; i++) {
            if ((len = strlen(args[i]->longname)) > maxlen)
                maxlen = len;
        }
    maxlen += 12;
    // indent + short + commaspace + longflag + long + spacing
    // 2        2       2            2          max    4
    for (i = 0; i < state->n_args; i++) {
        if (state->arg_pos_idx[pos_arg] == (int)i) {
            pos_arg++;
            continue;
        }
        arg = args[i];
        if (config->parse_long) {
            len = strlen(arg->longname);
            printf("  -%c, --%s", arg->shortname, arg->longname);
        } else {
            printf("  -%c", arg->shortname);
        }
        for (j = 0; j < maxlen - (8 + len); j++)
            printf(" ");
        printf("%s\n", arg->help);
    }
    if (state->n_subc > 0)
        printf("\nRun '%s COMMAND --help' for information about a specific "
               "subcommand\n",
               argv0);
}

// TODO: validate that all required arguments were provided
// TODO: minimal usage
void arg_print_error(int ret, arg_t *s, char **argv) {
    unsigned int printed = 0;
    printf("Error: ");
    switch (ret) {
    case ARG_ERR_CONFIG_TOO_MANY_ARGS:
        puts("Too many arguments given to cli config.\n");
        printed = 1;
        break;
    case ARG_ERR_CANNOT_ADD_HELP:
        puts("Cannot add help parameter, would exceed max arguments.\n");
        printed = 1;
        break;
    case ARG_ERR_TOO_MANY_SUBCOMMANDS:
        puts("Too many subcommands given to cli config.\n");
        printed = 1;
        break;
    case ARG_ERR_GENERIC:
        puts("Generic CLI parsing error occurred.\n");
        break;
    }
    if (printed) {
        puts("The above error is the responsibility of the program author.\n");
        return;
    }

    switch (ret) {
    case ARG_ERR_MISSING_ARGUMENT:
        printf("Option requires argument: -%c\n", argv[s->optind][s->optpos]);
        break;
    case ARG_ERR_MISSING_LONG_ARGUMENT:
        printf("Option requires argument: %s\n", argv[s->optind]);
        break;
    case ARG_ERR_UNKNOWN_ARGUMENT:
        printf("Unknown argument: -%c\n", argv[s->optind][s->optpos]);
        break;
    case ARG_ERR_UNKNOWN_LONG_ARGUMENT:
        printf("Unknown argument: %s\n", argv[s->optind]);
        break;
    case ARG_ERR_TOO_MANY_POSITIONALS:
        puts("Too many positional arguments provided.\n");
        break;
    case ARG_ERR_GENERIC:
        puts("Generic error encountered when parsing commands.\n");
        break;
    case ARG_ERR_NO_LONG_ARGS:
        puts("This program was not configured to accept long arguments.\n");
        break;
    case ARG_ERR_SUBC_REQUIRED:
        puts("A subcommand is required, none were provided.\n");
        break;
    }
    puts("See -h or --help for more details."); // TODO: only print when help
                                                // present?
}
