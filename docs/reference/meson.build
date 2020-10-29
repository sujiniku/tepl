subdir('xml')

configure_file(
  input: 'tepl-sections.txt',
  output: 'tepl-@0@-sections.txt'.format(TEPL_API_VERSION),
  copy: true
)

gtkdoc_module_name = 'tepl-@0@'.format(TEPL_API_VERSION)
html_dir = get_option('prefix') / GNOME.gtkdoc_html_dir(gtkdoc_module_name)

glib_docpath = dependency('glib-2.0').get_pkgconfig_variable('prefix') / 'share/gtk-doc/html/glib'
gobject_docpath = dependency('gobject-2.0').get_pkgconfig_variable('prefix') / 'share/gtk-doc/html/gobject'
gio_docpath = dependency('gio-2.0').get_pkgconfig_variable('prefix') / 'share/gtk-doc/html/gio'
gtk_docpath = dependency('gtk+-3.0').get_pkgconfig_variable('prefix') / 'share/gtk-doc/html/gtk3'
gsv_docpath = dependency('gtksourceview-4').get_pkgconfig_variable('prefix') / 'share/gtk-doc/html/gtksourceview-4.0'
amtk_docpath = dependency('amtk-5').get_pkgconfig_variable('prefix') / 'share/gtk-doc/html/amtk-5'

GNOME.gtkdoc(
  gtkdoc_module_name,
  main_xml: 'tepl-docs.xml',
  src_dir: include_directories('../../tepl/'),
  dependencies: TEPL_LIB_DEP,
  scan_args: ['--rebuild-types'],
  gobject_typesfile: 'tepl-@0@.types'.format(TEPL_API_VERSION),
  fixxref_args: [
    '--html-dir=@0@'.format(html_dir),
    '--extra-dir=@0@'.format(glib_docpath),
    '--extra-dir=@0@'.format(gobject_docpath),
    '--extra-dir=@0@'.format(gio_docpath),
    '--extra-dir=@0@'.format(gtk_docpath),
    '--extra-dir=@0@'.format(gsv_docpath),
    '--extra-dir=@0@'.format(amtk_docpath)
  ],
  content_files: [
    'api-breaks.xml',
    'intro.xml',
  ],
  ignore_headers: TEPL_PRIVATE_HEADERS,
  install: true
)
