interactive_tests = [
  # executable name, sources
  ['test-fold-region', 'test-fold-region.c'],
  ['test-language-chooser-dialog', 'test-language-chooser-dialog.c'],
  ['test-language-chooser-widget', 'test-language-chooser-widget.c'],
  ['test-space-drawer-prefs', 'test-space-drawer-prefs.c'],
  ['test-tab', 'test-tab.c'],

  ['test-gutter-renderer-folds',
   ['test-gutter-renderer-folds.c',
    'tepl-gutter-renderer-folds-sub.c']],
]

foreach test : interactive_tests
  executable(
    test[0], test[1],
    dependencies: TEPL_STATIC_DEP
  )
endforeach
