unit_tests = [
  'test-buffer-input-stream',
  'test-encoding',
  'test-encoding-converter',
  'test-file',
  'test-file-content',
  'test-file-loader',
  'test-file-metadata',
  'test-file-saver',
  'test-fold-region',
  'test-info-bar',
  'test-metadata-store',
  'test-notebook',
  'test-utils'
]

foreach test_name : unit_tests
  test_exe = executable(
    test_name,
    test_name + '.c',
    dependencies: TEPL_STATIC_DEP
  )
  test(test_name, test_exe)
endforeach
