unit_tests = [
  'test-file',
  'test-file-loader',
  'test-file-saver',
  'test-fold-region',
  'test-icu',
  'test-info-bar',
  'test-metadata',
  'test-metadata-manager',
  'test-notebook',
  'test-utils'
]

foreach test_name : unit_tests
  test_exe = executable(
    test_name,
    [test_name + '.c', 'tepl-test-utils.c'],
    dependencies: TEPL_STATIC_DEP,
    c_args: '-DUNIT_TESTS_SOURCE_DIR="@0@"'.format(meson.current_source_dir())
  )
  test(test_name, test_exe)
endforeach
