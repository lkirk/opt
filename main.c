#include <stdio.h>

#include "arg.h"

enum subcommands {
    CMD_1,
    CMD_2,
    N_CMD,
};

int subcommand_1_main(int argc, char **argv, arg_t *state) {
    int ret;
    arg_arg_t *arg;
    arg_config_t cli_config = {
        .parse_long = 1,
        .add_help = 1,
        .subcommand_required = 0,
        .description = "Descriptive description of subcommand 1",
        .subcommands = {NULL},
        .args = {ARG('o', "o-opt", ARG_REQUIRED | ARG_HAS_ARG,
                     "Required o parameter."),
                 NULL}};
    // TODO: chain multiple subc names in nested subcs... up to user?
    char *subc_name = state->optarg;

    char *o = "";
    arg_init(state, &cli_config, 1);
    while ((ret = arg_parse(argc, argv, state, &cli_config)) > 0) {
        switch (ret) {
        case ARG_OK:
            arg = cli_config.args[state->argind];
            switch (arg->shortname) {
            case 'o':
                o = state->optarg;
                break;
            case 'h':
                arg_usage(argv[0], state, subc_name, &cli_config);
                exit(1);
            }
            break;
        }
    }
    if (ret < 0) {
        arg_print_error(ret, state, argv);
        exit(1);
    }
    printf("Entering subcommand: %s\n", subc_name);
    printf("o=%s\n", o);
    return ret;
}

int subcommand_2_main(int argc, char **argv, arg_t *state) {
    int ret;
    arg_arg_t *arg;
    arg_config_t cli_config = {
        .parse_long = 1,
        .add_help = 1,
        .subcommand_required = 0,
        .description = "Descriptive description of subcommand 2",
        .subcommands = {NULL},
        .args = {ARG('f', "f-opt", 0, "Optional F flag (default false)."),
                 NULL}};
    // TODO: chain multiple subc names in nested subcs... up to user?
    char *subc_name = state->optarg;

    unsigned int f = 0;
    arg_init(state, &cli_config, 1);
    while ((ret = arg_parse(argc, argv, state, &cli_config)) > 0) {
        arg = cli_config.args[state->argind];
        switch (ret) {
        case ARG_OK:
            switch (arg->shortname) {
            case 'f':
                f = 1;
                break;
            case 'h':
                arg_usage(argv[0], state, subc_name, &cli_config);
                exit(1);
            }
            break;
        }
    }
    if (ret < 0) {
        arg_print_error(ret, state, argv);
        exit(1);
    }
    printf("Entering subcommand: %s\n", subc_name);
    printf("f=%u\n", f);
    return ret;
}

int main(int argc, char **argv) {
    int ret;
    arg_t state;
    arg_arg_t *arg;

    char *a = "", *b = "";
    unsigned int c = 0, d = 0;
    unsigned int subc_found = 0;
    arg_config_t cli_config = {
        .parse_long = 1,
        .add_help = 1,
        .subcommand_required = 1,
        .description = "An example command line interface for testing",
        .subcommands = {"command1", "command2", NULL},
        .args = {
            ARG('a', "a-opt", ARG_REQUIRED | ARG_HAS_ARG,
                "This is a required option, providing the A "
                "parameter."),
            ARG('b', "b-flag", ARG_REQUIRED,
                "This is an optional arg, providing the B parameter."),
            ARG('c', "c-opt-flag", 0,
                "This is the optional C flag (default false)."),
            ARG('d', "d-opt-flag", 0,
                "This is the optional D flag (default false)."),
            POSITIONAL_ARG("pos_1", 0, "First positional arg"),
            POSITIONAL_ARG("pos_2", 0, "Second positional arg"),
            NULL,
        }};

    arg_init(&state, &cli_config, 0);
    while ((ret = arg_parse(argc, argv, &state, &cli_config)) > 0) {
        arg = cli_config.args[state.argind];
        switch (ret) {
        case ARG_OK: {
            switch (arg->shortname) {
            case 'a':
                a = state.optarg;
                break;
            case 'b':
                b = state.optarg;
                break;
            case 'c':
                c = 1;
                break;
            case 'd':
                d = 1;
                break;
            case 'h':
                arg_usage(argv[0], &state, NULL, &cli_config);
                exit(1);
            }
            break;
        }
        case ARG_SUBC:
            switch (state.subc_idx) {
            case CMD_1:
                subcommand_1_main(argc, argv, &state);
                subc_found = 1;
                break;
            }
            break;
        }
    }
    if (cli_config.subcommand_required && !subc_found)
        ret = ARG_ERR_SUBC_REQUIRED;

    if (ret < 0) {
        arg_print_error(ret, &state, argv);
        return 1;
    }
    // if (subc_ret < 0) {
    //     // handle subc runtime err
    // }
    printf("a=%s\nb=%s\nc=%u\nd=%u\n", a, b, c, d);
    return 0;
}
