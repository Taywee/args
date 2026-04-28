project('args.hxx', ['cpp'],
  version: '6.4.14',
  default_options: 'cpp_std=c++11',
  license: 'MIT'
)

# This is a header-only lib, all we need to do is include it
args_dep = declare_dependency(
  include_directories: include_directories('.')
).as_system('system')

install_headers('args.hxx')

# examples
if get_option('examples')
executable('gitlike', sources: 'examples/gitlike.cxx', dependencies: args_dep)
executable('completion', sources: 'examples/completion.cxx', dependencies: args_dep)
endif

# tests
if get_option('unittests')
test_names = [
  'action_flag',
  'arg_flag_lists',
  'arg_flag_lists_default',
  'arg_flag_lists_replace_default',
  'arg_flag_lists_sets',
  'arg_flags',
  'arg_to_non_arg_flag',
  'argument_separation',
  'bool_flags',
  'choices_description',
  'completion',
  'completion_bad_cword',
  'count_flag',
  'custom_types',
  'default_values',
  'extra_positionals',
  'get_assignable',
  'get_program_line',
  'global_options',
  'group_nesting',
  'group_validation_fail',
  'help_flag',
  'help_params',
  'help_stream',
  'help_underflow',
  'hidden_options',
  'implicit_values',
  'integer_overflow',
  'invalid_arg_parse',
  'kickout_flags',
  'mapping_types',
  'matcher_validation',
  'nargs',
  'noexcept_completion',
  'noexcept_completion_bad_cword',
  'noexcept_matcher_validation',
  'noexcept_mode',
  'noexcept_nargs',
  'noexcept_required_flags',
  'noexcept_subparser_validation',
  'noexcept_unsigned_negative',
  'option_terminator',
  'positional_and_lists',
  'positional_lists',
  'positional_lists_default',
  'positional_lists_replace_default',
  'positional_lists_sets',
  'positional_unspecified',
  'prefix_dd_style',
  'prefix_windows_style',
  'program_line_wrap',
  'required_flags',
  'simple_commands',
  'single_flag_repeat',
  'subparser_commands',
  'subparser_commands_kickout',
  'subparser_group_validation',
  'subparser_help',
  'subparser_kickout',
  'subparser_validation',
  'unified_match_lists',
  'unknown_flags',
  'unsigned_negative',
  'value_parser',
  'windows_h',
]

foreach t : test_names
  test(t, executable('argstest-' + t,
    sources: 'test' / (t + '.cxx'),
    dependencies: args_dep))
endforeach

test('multiple_inclusion', executable('argstest-multiple_inclusion',
  sources: ['test/multiple_inclusion.cxx', 'test/multiple_inclusion.aux.cxx'],
  dependencies: args_dep))
endif
