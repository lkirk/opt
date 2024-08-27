// TODO: rename whole lib to "opt"
#include <stdio.h> // for main, remove when lib
#include <stdlib.h>
#include <string.h>
#define MAX_ARGS 30 // maximum number of arguments (per subcommand + main)
#define MAX_SUBC 10
#define MAX_SUBC_LEN 256
#define MAX_ARG_LEN 512

// these are the values we accept as short arguments
static const char ascii[128] = {
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', 'A',  'B',  'C',  'D',  'E',  'F',  'G',
    'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',
    'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '\0', '\0', '\0', '\0', '\0',
    '\0', 'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',
    'l',  'm',  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
    'x',  'y',  'z',  '\0', '\0', '\0', '\0', '\0'};

// TODO: all max checks... should we add +1 to the max?

#define ARG_REQUIRED (1U << 1)
#define ARG_POSITIONAL (1U << 2)
#define ARG_HAS_ARG (1U << 3)

typedef struct arg_arg {
    char shortname;
    char *longname;
    unsigned int type;
    char *help;
} arg_arg_t;

typedef struct arg_config {
    int parse_long;
    const char *subcommands[MAX_SUBC];
    arg_arg_t *args[MAX_ARGS];
    arg_arg_t *subcommand_args[MAX_SUBC][MAX_ARGS];
} arg_config_t;

typedef struct arg {
    // external API
    int optopt;
    int optind;
    int optpos;
    char *optarg;

    // internal API
    unsigned int n_pos_seen;   // Number of positional args encountered
    unsigned int n_spos_seen;  // Number of subc pos args encountered
    unsigned int argind;       // Index of parsed argument
    unsigned int has_subc;     // Are we configured to use subc?
    unsigned int in_subc;      // Currently parsing a subcommand
    unsigned int subc_idx;     // Current index of found subarg
    unsigned int n_args;       // Number of arguments provided (not inc pos)
    unsigned int n_arg_pos;    // Num positional args
    int arg_pos_idx[MAX_ARGS]; // Positional arg idx
    unsigned int n_subc;       // Number of subc
    unsigned int n_subc_args[MAX_SUBC];   // Number of args per subc
    unsigned int n_subc_pos[MAX_SUBC];    // Num positional args for each subc
    int subc_pos_idx[MAX_SUBC][MAX_ARGS]; // Subc positional arg idx
    int arg_idx[128];
    int sarg_idx[MAX_SUBC][128];
} arg_t;

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
    for (i = 0; subcs[i] && i < MAX_SUBC; i++)
        if (!strncmp(s, subcs[i], MAX_SUBC_LEN))
            return 1;
    return 0;
}

unsigned int ascii_idx(char c) { return ascii[(int)(c)]; }

// void count_types(arg_t *state, arg_arg_t *arg) {
//     state->n_args++;
//     if (arg->type & ARG_POSITIONAL)
//         state->n_arg_pos++;
//     if (arg->type & ARG_
// }

int arg_init(arg_t *state, arg_config_t *config) {
    // validate and initialize parsing state
    int i, j;
    arg_arg_t **sargs, **args = config->args;

    memset(state, 0, sizeof(*state));
    // n_* get set to 0, *_idx get set to -1
    memset(state->n_subc_args, 0, sizeof(*state->n_subc_args) * MAX_SUBC);
    memset(state->n_subc_pos, 0, sizeof(*state->n_subc_pos) * MAX_SUBC);
    memset(state->arg_pos_idx, -1, sizeof(*state->arg_pos_idx) * MAX_ARGS);
    memset(state->subc_pos_idx, -1,
           sizeof(**state->subc_pos_idx) * MAX_ARGS * MAX_SUBC);
    memset(state->arg_idx, -1, sizeof(*state->arg_idx) * 128);
    memset(state->sarg_idx, -1, sizeof(**state->sarg_idx) * 128 * MAX_SUBC);
    state->optind = 1;
    state->optpos = 1;

    for (i = 0; args[i] && i < MAX_ARGS; i++)
        if (args[i]->type & ARG_POSITIONAL)
            state->n_arg_pos++;
        else
            state->arg_idx[ascii_idx(args[i]->shortname)] = i;
    state->n_args = i;
    if (i == MAX_ARGS)
        return -1; // too many args or no null terminator

    for (i = 0; *(sargs = config->subcommand_args[i]) && i < MAX_SUBC; i++) {
        for (j = 0; sargs[j] && j < MAX_ARGS; j++)
            if (sargs[j]->type & ARG_POSITIONAL)
                state->n_subc_pos[i]++;
            else
                state->sarg_idx[i][ascii_idx(sargs[j]->shortname)] = j;
        state->n_subc_args[i] = j;
    }
    if (i == MAX_SUBC)
        return -1; // too many subc or no null terminator
    for (j = 0; config->subcommands[j] && j < MAX_SUBC; j++)
        ;
    if (j != i)
        return -1; // all subcs must have arg struct (even if empty)
    if (j > 0) {
        state->has_subc = 1;
        state->n_subc = j;
    }
    return 0;
}

#define ARG(s, l, flag, help) (&(arg_arg_t){s, l, flag, help})
#define POSITIONAL_ARG(name, help)                                             \
    (&(arg_arg_t){'\0', name, ARG_POSITIONAL, help})

// TODO: arg string msg.

enum arg_ret {
    ARG_ERR = -1,
    ARG_DONE,
    ARG_ARG,
    ARG_FLAG,
    ARG_SUBC,
    ARG_POS,
};

// TODO: this should return the arg type (long, short, subc, etc...) as an enum
// value. consumer will switch on enum val type (see above enum).
// TODO: use optopt???
int arg_parse(int argc, char **argv, arg_t *s, arg_config_t *config) {
    int i, n_args, *arg_idx;
    arg_arg_t *arg = NULL, **args;
    char *arg_str;
    unsigned int arg_len, *n_pos_seen, *n_pos;

    if (s->optind == argc)
        return ARG_DONE; // TODO: validate that all required args were found
    arg_str = argv[s->optind];
    if (s->in_subc) {
        args = config->subcommand_args[s->subc_idx];
        n_args = s->n_subc_args[s->subc_idx];
        n_pos_seen = &s->n_spos_seen;
        n_pos = &s->n_subc_pos[s->subc_idx];
        arg_idx = s->sarg_idx[s->subc_idx];
    } else {
        args = config->args;
        n_args = s->n_args;
        n_pos_seen = &s->n_pos_seen;
        n_pos = &s->n_arg_pos;
        arg_idx = s->arg_idx;
    }

    if (!arg_str) {
        return ARG_ERR;

    } else if (is_doubledash(arg_str)) {
        // TODO: look in more detail into what happens after double dash
        s->optind++;
        return ARG_ERR;

    } else if (is_short(arg_str)) { // TODO: need to test alnum before indexing
        if ((i = arg_idx[ascii_idx(arg_str[s->optpos])]) == -1)
            return ARG_ERR; // arg unknown... err str... keep parsing?
        s->argind = i;
        arg = args[i];
        if (arg->type & ARG_REQUIRED) {
            s->optpos++;
            if (!arg_str[s->optpos] && s->optind == argc - 1) // this is the end
                // TODO: continue parsing if no arg (not at end)?
                return ARG_ERR; // Needed argument
            if (!arg_str[s->optpos])
                s->optarg =
                    argv[++s->optind]; // TODO: validate this is not a flag?
            else if (arg_str[s->optpos] == '=')
                s->optarg = &arg_str[++s->optpos];
            else
                s->optarg = &arg_str[s->optpos];
            s->optpos = 1;
            s->optind++;
            return ARG_ARG;
        }
        if (!arg_str[++s->optpos]) {
            s->optind++;
            s->optpos = 1;
        }
        return ARG_FLAG;

    } else if (is_long(arg_str)) { // Long argument
        s->optpos = 2;
        if (!config->parse_long)
            return ARG_ERR; // Didn't expect long args
        for (i = 0; i < n_args; i++) {
            arg_len = strlen(args[i]->longname);
            if (!strncmp(args[i]->longname, &arg_str[s->optpos], arg_len)) {
                arg = args[i];
                break;
            }
        }
        if (!arg)
            return ARG_ERR; // Unexpected argument
        s->argind = i;
        if (arg->type & ARG_REQUIRED) {
            s->optpos += arg_len; // advance to end of arg
            if (!arg_str[s->optpos] && s->optind == argc - 1) // this is the end
                // TODO: continue parsing if no arg (not at end)?
                return ARG_ERR; // Needed argument
            if (!arg_str[s->optpos])
                s->optarg =
                    argv[++s->optind]; // TODO: validate this is not a flag?
            else if (arg_str[s->optpos] == '=')
                s->optarg = &arg_str[++s->optpos];
            else
                s->optarg = &arg_str[s->optpos];
            s->optpos = 1;
            s->optind++;
            return ARG_ARG;
        }
        s->optind++;
        s->optpos = 1;
        return ARG_FLAG;

    } else { // must be a positional arg or subcommand
        for (i = 0; i < (int)s->n_subc; i++)
            if (!strncmp(arg_str, config->subcommands[i], strlen(arg_str)))
                break;
        if (i == (int)s->n_subc) { // not matched, must be positional
            if (*n_pos_seen == *n_pos) {
                return ARG_ERR; // unrecognized argument, we've seen all
                                // expected pos args
            } else {
                (*n_pos_seen)++;
                s->optarg = arg_str;
                s->optind++;
                return ARG_POS;
            }
        } else {
            if (s->in_subc)
                return ARG_ERR; // already parsed subcommand
            s->in_subc = 1;
            s->subc_idx = i;
            s->optarg = arg_str;
            s->optind++;
            return ARG_SUBC;
        }
    }
    return ARG_ERR; // unexpected parsing state
}

enum subcommands {
    CMD_1,
    CMD_2,
    N_CMD,
};

int main(int argc, char **argv) {
    arg_t state;
    int ret;
    unsigned int arg_idx;
    arg_arg_t *arg;
    char *optarg;
    arg_config_t cli_config = {
        .parse_long = 1,
        .subcommands = {"command1", "command2", NULL},
        .args =
            {
                ARG('a', "a-opt", ARG_REQUIRED | ARG_HAS_ARG,
                    "This is a required option, providing the A parameter."),
                ARG('b', "b-flag", ARG_REQUIRED,
                    "This is an optional arg, providing the B flag."),
                ARG('c', "c-opt-flag", 0, "This is the optional C flag."),
                ARG('d', "d-opt-flag", 0, "This is the optional D flag."),
                POSITIONAL_ARG("pos_1", "First positional arg"),
                POSITIONAL_ARG("pos_2", "Second positional arg"),
                NULL,
            },
        .subcommand_args = {
            {
                ARG('o', "o-opt", ARG_REQUIRED | ARG_HAS_ARG,
                    "Required o parameter for command1."),
                NULL,
            },
            {
                ARG('f', "f-opt", 0, "Optional F flag for command2."),
                NULL,
            },
            {NULL}}};

    arg_init(&state, &cli_config);
    while ((ret = arg_parse(argc, argv, &state, &cli_config))) {
        arg_idx = state.argind;
        optarg = state.optarg;
        switch (ret) {
        case ARG_ERR:
            goto out;
        case ARG_DONE:
            goto out;
        case ARG_FLAG: {
            if (state.in_subc)
                arg = cli_config.subcommand_args[state.subc_idx][arg_idx];
            else
                arg = cli_config.args[arg_idx];
            printf("got%s flag %c\n", state.in_subc ? " subc" : "",
                   arg->shortname);
            break;
        }
        case ARG_ARG: {
            if (state.in_subc)
                arg = cli_config.subcommand_args[state.subc_idx][arg_idx];
            else
                arg = cli_config.args[arg_idx];
            printf("got%s arg %c with value %s\n", state.in_subc ? " subc" : "",
                   arg->shortname, optarg);
            break;
        }
        case ARG_SUBC: {
            printf("got subcommand %s\n", optarg);
            break;
        }
        case ARG_POS:
            if (state.in_subc)
                printf("got subc pos arg with value %s\n", optarg);
            else
                printf("got pos arg with value %s\n", optarg);
            break;
        }
    }
out:
    return ret;
}
