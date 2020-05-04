tepl_public_headers = [
  'tepl.h',
  'tepl-abstract-factory.h',
  'tepl-application.h',
  'tepl-application-window.h',
  'tepl-buffer.h',
  'tepl-encoding.h',
  'tepl-file.h',
  'tepl-file-loader.h',
  'tepl-file-saver.h',
  'tepl-fold-region.h',
  'tepl-goto-line-bar.h',
  'tepl-gutter-renderer-folds.h',
  'tepl-info-bar.h',
  'tepl-init.h',
  'tepl-io-error-info-bars.h',
  'tepl-iter.h',
  'tepl-macros.h',
  'tepl-menu-shell.h',
  'tepl-metadata.h',
  'tepl-metadata-manager.h',
  'tepl-notebook.h',
  'tepl-panel.h',
  'tepl-statusbar.h',
  'tepl-style-scheme-chooser-widget.h',
  'tepl-tab.h',
  'tepl-tab-group.h',
  'tepl-tab-label.h',
  'tepl-utils.h',
  'tepl-view.h'
]

tepl_public_c_files = [
  'tepl-abstract-factory.c',
  'tepl-application.c',
  'tepl-application-window.c',
  'tepl-buffer.c',
  'tepl-encoding.c',
  'tepl-file.c',
  'tepl-file-loader.c',
  'tepl-file-saver.c',
  'tepl-fold-region.c',
  'tepl-goto-line-bar.c',
  'tepl-gutter-renderer-folds.c',
  'tepl-info-bar.c',
  'tepl-init.c',
  'tepl-io-error-info-bars.c',
  'tepl-iter.c',
  'tepl-menu-shell.c',
  'tepl-metadata.c',
  'tepl-metadata-manager.c',
  'tepl-notebook.c',
  'tepl-panel.c',
  'tepl-statusbar.c',
  'tepl-style-scheme-chooser-widget.c',
  'tepl-tab.c',
  'tepl-tab-group.c',
  'tepl-tab-label.c',
  'tepl-utils.c',
  'tepl-view.c'
]

TEPL_PRIVATE_HEADERS = [
  'tepl-buffer-input-stream.h',
  'tepl-close-confirm-dialog-single.h',
  'tepl-encoding-converter.h',
  'tepl-encoding-private.h',
  'tepl-file-content.h',
  'tepl-file-content-loader.h',
  'tepl-io-error-info-bar.h',
  'tepl-metadata-attic.h',
  'tepl-metadata-parser.h',
  'tepl-progress-info-bar.h',
  'tepl-signal-group.h',
  'tepl-tab-saving.h',
  'tepl-window-actions-edit.h',
  'tepl-window-actions-file.h',
  'tepl-window-actions-search.h'
]

tepl_private_c_files = [
  'tepl-buffer-input-stream.c',
  'tepl-close-confirm-dialog-single.c',
  'tepl-encoding-converter.c',
  'tepl-file-content.c',
  'tepl-file-content-loader.c',
  'tepl-io-error-info-bar.c',
  'tepl-metadata-attic.c',
  'tepl-metadata-parser.c',
  'tepl-progress-info-bar.c',
  'tepl-signal-group.c',
  'tepl-tab-saving.c',
  'tepl-window-actions-edit.c',
  'tepl-window-actions-file.c',
  'tepl-window-actions-search.c'
]

headers_install_dir = get_option('includedir') / 'tepl-@0@/tepl/'.format(TEPL_API_VERSION)
install_headers(
  tepl_public_headers,
  install_dir: headers_install_dir
)

tepl_enum_types = GNOME.mkenums_simple(
  'tepl-enum-types',
  decorator: '_TEPL_EXTERN',
  header_prefix: '#include <tepl/tepl-macros.h>',
  sources: tepl_public_headers,
  install_header: true,
  install_dir: headers_install_dir
)

tepl_static_lib_c_args = [ '-DTEPL_COMPILATION' ]
if meson.get_compiler('c').get_argument_syntax() == 'msvc'
  tepl_static_lib_c_args += '-D_TEPL_EXTERN=__declspec(dllexport) extern'
endif

tepl_static_lib = static_library(
  'tepl-static',
  [tepl_public_c_files,
   tepl_private_c_files,
   tepl_enum_types],
  pic: true, # tepl_static_lib is linked in a shared library.
  include_directories: ROOT_INCLUDE_DIR,
  dependencies: TEPL_DEPS,
  c_args: tepl_static_lib_c_args
)

# For unit tests, to be able to test private functions.
TEPL_STATIC_DEP = declare_dependency(
  include_directories: ROOT_INCLUDE_DIR,
  link_with: tepl_static_lib,
  sources: tepl_enum_types[1],
  dependencies: TEPL_DEPS
)

tepl_lib_link_args = []
tepl_lib_link_depends = []

if meson.get_compiler('c').get_argument_syntax() != 'msvc'
  symbol_map = meson.current_source_dir() / 'symbol.map'
  tepl_lib_link_args = '-Wl,--version-script,' + symbol_map
  tepl_lib_link_depends = symbol_map
endif

tepl_lib = library(
  'tepl-@0@'.format(TEPL_API_VERSION),
  dependencies: TEPL_DEPS,
  link_args: tepl_lib_link_args,
  link_depends: tepl_lib_link_depends,
  # link_whole is not supported with MSVC, so we use extract_all_objects().
  objects: tepl_static_lib.extract_all_objects(),
  version: TEPL_LT_VERSION,
  install: true
)

TEPL_LIB_DEP = declare_dependency(
  include_directories: ROOT_INCLUDE_DIR,
  link_with: tepl_lib,
  sources: tepl_enum_types[1],
  dependencies: TEPL_DEPS
)

# pkg-config file generation. Supports GtkSourceView being built as a
# subproject.
PKG_CONFIG.generate(tepl_lib,
  filebase: 'tepl-@0@'.format(TEPL_API_VERSION),
  name: 'Tepl',
  description: 'Text editor product line',
  subdirs: 'tepl-@0@'.format(TEPL_API_VERSION),
  libraries: TEPL_PUBLIC_DEPS,
)

GNOME.generate_gir(
  tepl_lib,
  export_packages: 'tepl-@0@'.format(TEPL_API_VERSION),
  header: 'tepl/tepl.h',
  identifier_prefix: 'Tepl',
  include_directories: ROOT_INCLUDE_DIR,
  includes: ['Gtk-3.0', 'GtkSource-4', 'Amtk-5'],
  install: true,
  namespace: 'Tepl',
  nsversion: TEPL_API_VERSION,
  sources: [
    tepl_public_headers,
    tepl_public_c_files,
    tepl_enum_types
  ],
  # Support for deps being built as subprojects:
  dependencies: TEPL_DEPS,
)
