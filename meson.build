# Convention:
# - Local variables in lower_case.
# - Global variables in UPPER_CASE.
# See: https://github.com/mesonbuild/meson/issues/2607

project(
  'tepl', 'c',
  meson_version: '>= 0.53',
  version: '5.99.0',
  default_options: ['warning_level=2']
)

GNOME = import('gnome')
PKG_CONFIG = import('pkgconfig')
I18N = import('i18n')

# Libtool versioning
#
# For development releases (if the minor package version is odd), keep the same
# Libtool version.
#
# For a new minor stable release (when incrementing the minor package version
# to an even number), apply the following algorithm step by step:
# 1. If the library source code has changed at all since the last
#    update, then increment REVISION.
# 2. If any exported functions or data have been added, removed, or
#    changed since the last update, increment CURRENT and set REVISION
#    to 0.
# 3. If any exported functions or data have been added since the last
#    public release, increment AGE.
# 4. If any exported functions or data have been removed since the last
#    public release, set AGE to 0.
#
# When incrementing the API version (usually for a new major package version),
# set CURRENT, REVISION and AGE to 0 since it's like a new library.
lt_current = 0
lt_revision = 0
lt_age = 0
TEPL_LT_VERSION = '@0@.@1@.@2@'.format(lt_current, lt_revision, lt_age)

# API version, used for parallel installability.
TEPL_API_VERSION = '6'

TEPL_PUBLIC_DEPS = [
  dependency('gio-2.0', version: '>= 2.62'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('gtksourceview-4', version: '>= 4.0'),
  dependency('amtk-5', version: '>= 5.0')
]
tepl_private_deps = [
  dependency('icu-uc'),
  dependency('icu-i18n'),
]
TEPL_DEPS = [TEPL_PUBLIC_DEPS, tepl_private_deps]

# config.h

config_data = configuration_data()
GETTEXT_PACKAGE_NAME = 'tepl-' + TEPL_API_VERSION
config_data.set_quoted('GETTEXT_PACKAGE', GETTEXT_PACKAGE_NAME)
config_data.set_quoted('TEPL_LOCALEDIR', get_option('prefix') / get_option('localedir'))

configure_file(
  output: 'config.h',
  configuration: config_data
)

# Misc

ROOT_INCLUDE_DIR = include_directories('.')

add_project_arguments(
  '-DG_LOG_DOMAIN="@0@"'.format(meson.project_name()),
  language: 'c'
)

#####
# CFLAGS
# Some flags are missing when using only the builtin warning_level meson option,
# even at the maximum level.
# The following warning_cflags suppose that warning_level=2.

c_compiler = meson.get_compiler('c')
warning_cflags = []

if c_compiler.get_id() == 'msvc'
  # Use GLib's msvc_recommended_pragmas.h to filter out
  # the warnings we don't really need to worry about,
  # but do make the ones we want to be wary stand out
  warning_cflags += [
    '-FImsvc_recommended_pragmas.h',
  ]
else
  # Try to mimic the AX_COMPILER_FLAGS Autotools macro.
  warning_cflags += [
    '-fno-strict-aliasing',
    '-Wundef',
    '-Wnested-externs',
    '-Wwrite-strings',
    '-Wpointer-arith',
    '-Wmissing-declarations',
    '-Wmissing-prototypes',
    '-Wstrict-prototypes',
    '-Wredundant-decls',
    '-Wno-unused-parameter',
    '-Wno-missing-field-initializers',
    '-Wdeclaration-after-statement',
    '-Wformat=2',
    '-Wold-style-definition',
    '-Wcast-align',
    '-Wformat-nonliteral',
    '-Wformat-security',
    '-Wsign-compare',
    '-Wstrict-aliasing',
    '-Wshadow',
    '-Winline',
    '-Wpacked',
    '-Wmissing-format-attribute',
    '-Wmissing-noreturn',
    '-Winit-self',
    '-Wredundant-decls',
    '-Wmissing-include-dirs',
    '-Wunused-but-set-variable',
    '-Warray-bounds',
    '-Wimplicit-function-declaration',
    '-Wreturn-type',
    '-Wswitch-enum',
    '-Wswitch-default',
    '-Wduplicated-cond',
    '-Wduplicated-branches',
    '-Wlogical-op',
    '-Wrestrict',
    '-Wnull-dereference',
    '-Wjump-misses-init',
    '-Wdouble-promotion'
  ]
endif

supported_warning_cflags = c_compiler.get_supported_arguments(warning_cflags)
add_project_arguments(supported_warning_cflags, language: 'c')
##### end CFLAGS

subdir('po')
subdir('tepl')
subdir('tests')
subdir('testsuite')

if get_option('gtk_doc')
  subdir('docs/reference')
endif

summary('API version', TEPL_API_VERSION)
summary('Prefix', get_option('prefix'))
summary('API documentation', get_option('gtk_doc'))
