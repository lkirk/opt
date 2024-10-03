project(
  'opt', 'c',
  version: run_command(
    'git', 'describe', '--always', '--dirty', check: false
  ).stdout().strip(),
  default_options: ['c_std=c99', 'default_library=static']
)

c_args = [
  '-Wall',
  '-Wextra',
  '-Wpedantic',
  '-Werror',
  '-Werror=vla',
  '-pedantic-errors',
  '-DARG_VERSION="@0@"'.format(meson.project_version())
]
inc = include_directories('.')

lib = library(
  'opt',
  'opt.c',
  include_directories: inc,
  version: '0.1',
  soversion: '1',
  install: true
)
opt_dep = declare_dependency(
  include_directories: inc,
  link_with: lib,
  version: meson.project_version()
)

if not meson.is_subproject()
  test_c_args = ['-DARG_TEST']
  cmocka = dependency('cmocka')
  test_init = executable(
    'test_init',
    include_directories: inc,
    dependencies: cmocka,
    sources: ['opt.c', 'tests/test_init.c']
  )
  test(
    'test_init',
    test_init,
    env: 'CMOCKA_MESSAGE_OUTPUT=tap',
    protocol: 'tap',
    verbose: true
  )
  executable(
    'ex_subcommands',
    include_directories: inc,
    link_with: lib,
    sources: 'examples/subcommands.c'
  )
endif
