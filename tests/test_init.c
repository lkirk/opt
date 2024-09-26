#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <cmocka.h>

#include "opt.h"

void test_opt_macro(void **OPT_UNUSED(state)) {
    opt_arg_t *arg = ARG('a', "a-opt", OPT_REQUIRED | OPT_HAS_ARG, "test");
    opt_arg_t *expected =
        &(opt_arg_t){'a', "a-opt", OPT_REQUIRED | OPT_HAS_ARG, "test"};
    assert_int_equal(arg->shortname, expected->shortname);
    assert_int_equal(arg->type, expected->type);
    assert_string_equal(arg->longname, expected->longname);
    assert_string_equal(arg->help, expected->help);
    // make sure we haven't added any fields
    assert_int_equal(32, sizeof(opt_arg_t));
}

void test_no_subc_no_help(void **OPT_UNUSED(state)) {
    opt_t s, expected;
    opt_config_t conf = {
        .parse_long = 1,
        .add_help = 0,
        .subcommand_required = 0,
        .description = "test",
        .subcommands = {NULL},
        .args = {ARG('a', "a-opt", OPT_REQUIRED | OPT_HAS_ARG, "Required opt"),
                 NULL}};
    opt_init(&s, &conf, 0);
    memset(&expected, 0, sizeof(expected));
    memset(&(&expected)->pos_idx, -1,
           sizeof(*(&expected)->pos_idx) * OPT_MAX_ARGS);
    expected.optind = 1;
    expected.optpos = 1;
    expected.n_args = 1;

    assert_memory_equal(&expected, &s, sizeof(s));
}

void test_no_subc_with_help(void **OPT_UNUSED(state)) {
    opt_t s, expected;
    opt_config_t conf = {
        .parse_long = 1,
        .add_help = 1,
        .subcommand_required = 0,
        .description = "test",
        .subcommands = {NULL},
        .args = {ARG('a', "a-opt", OPT_REQUIRED | OPT_HAS_ARG, "Required opt"),
                 NULL}};
    opt_arg_t *expected_args[3] = {
        ARG('a', "a-opt", OPT_REQUIRED | OPT_HAS_ARG, "Required opt"),
        ARG('h', "help", 0, "Print help message and exit."),
        NULL,
    };
    opt_init(&s, &conf, 0);
    memset(&expected, 0, sizeof(expected));
    memset(&(&expected)->pos_idx, -1,
           sizeof(*(&expected)->pos_idx) * OPT_MAX_ARGS);
    expected.optind = 1;
    expected.optpos = 1;
    expected.n_args = 2;

    assert_memory_equal(&expected, &s, sizeof(s));
    assert_int_equal(conf.args[0]->shortname, expected_args[0]->shortname);
    assert_int_equal(conf.args[1]->shortname, expected_args[1]->shortname);
    // last element should be null
    assert_int_equal(conf.args[2], expected_args[2]);
}

void test_subc_with_help(void **OPT_UNUSED(state)) {
    opt_t s, expected;
    opt_config_t conf = {
        .parse_long = 1,
        .add_help = 1,
        .subcommand_required = 1,
        .description = "test",
        .subcommands = {"command1", NULL},
        .args = {ARG('a', "a-opt", OPT_REQUIRED | OPT_HAS_ARG, "Required opt"),
                 NULL}};
    opt_arg_t *expected_args[3] = {
        ARG('a', "a-opt", OPT_REQUIRED | OPT_HAS_ARG, "Required opt"),
        ARG('h', "help", 0, "Print help message and exit."),
        NULL,
    };
    opt_init(&s, &conf, 0);
    memset(&expected, 0, sizeof(expected));
    memset(&(&expected)->pos_idx, -1,
           sizeof(*(&expected)->pos_idx) * OPT_MAX_ARGS);
    expected.optind = 1;
    expected.optpos = 1;
    expected.n_args = 2;
    expected.n_subc = 1;
    expected.has_subc = 1;

    assert_memory_equal(&expected, &s, sizeof(s));
    assert_int_equal(conf.args[0]->shortname, expected_args[0]->shortname);
    assert_int_equal(conf.args[1]->shortname, expected_args[1]->shortname);
    // last element should be null
    assert_int_equal(conf.args[2], expected_args[2]);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_opt_macro),
        cmocka_unit_test(test_no_subc_no_help),
        cmocka_unit_test(test_no_subc_with_help),
        cmocka_unit_test(test_subc_with_help),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
