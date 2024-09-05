project('arg', 'c', default_options: ['c_std=c99', 'default_library=static'])

version = run_command(
  'git', 'describe', '--always', '--dirty', check: false).stdout().strip()

c_args = [
  '-Wall', '-Wextra', '-Wpedantic', '-Werror',
  '-DARG_VERSION="@0@"'.format(version)
]

library('arg', 'arg.c', version: '0.1', soversion: '1')

test_arg = executable('test_arg', sources: ['arg.c', 'main.c'])
test('test_arg', test_arg)
