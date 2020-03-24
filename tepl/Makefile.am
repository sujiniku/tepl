include $(top_srcdir)/aminclude_static.am
clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean

AM_CPPFLAGS =				\
	-DDATADIR=\""$(datadir)"\"	\
	-DG_LOG_DOMAIN=\"Tepl\"		\
	-DTEPL_COMPILATION		\
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	$(WARN_CFLAGS)			\
	$(CODE_COVERAGE_CPPFLAGS)	\
	$(TEPL_DEP_CFLAGS)

tepl_public_headers =				\
	tepl.h					\
	tepl-abstract-factory.h			\
	tepl-abstract-factory-vala.h		\
	tepl-application.h			\
	tepl-application-window.h		\
	tepl-buffer.h				\
	tepl-encoding.h				\
	tepl-file.h				\
	tepl-file-loader.h			\
	tepl-file-metadata.h			\
	tepl-file-saver.h			\
	tepl-fold-region.h			\
	tepl-gutter-renderer-folds.h		\
	tepl-info-bar.h				\
	tepl-init.h				\
	tepl-io-error-info-bars.h		\
	tepl-iter.h				\
	tepl-menu-shell.h			\
	tepl-metadata-manager.h			\
	tepl-notebook.h				\
	tepl-tab.h				\
	tepl-tab-group.h			\
	tepl-tab-label.h			\
	tepl-types.h				\
	tepl-utils.h				\
	tepl-view.h

tepl_public_c_files =				\
	tepl-abstract-factory.c			\
	tepl-abstract-factory-vala.c		\
	tepl-application.c			\
	tepl-application-window.c		\
	tepl-buffer.c				\
	tepl-encoding.c				\
	tepl-file.c				\
	tepl-file-loader.c			\
	tepl-file-metadata.c			\
	tepl-file-saver.c			\
	tepl-fold-region.c			\
	tepl-gutter-renderer-folds.c		\
	tepl-info-bar.c				\
	tepl-init.c				\
	tepl-io-error-info-bars.c		\
	tepl-iter.c				\
	tepl-menu-shell.c			\
	tepl-metadata-manager.c			\
	tepl-notebook.c				\
	tepl-tab.c				\
	tepl-tab-group.c			\
	tepl-tab-label.c			\
	tepl-utils.c				\
	tepl-view.c

tepl_private_headers =				\
	tepl-buffer-input-stream.h		\
	tepl-close-confirm-dialog-single.h	\
	tepl-encoding-converter.h		\
	tepl-encoding-private.h			\
	tepl-file-content.h			\
	tepl-file-content-loader.h		\
	tepl-io-error-info-bar.h		\
	tepl-progress-info-bar.h		\
	tepl-signal-group.h			\
	tepl-tab-saving.h

tepl_private_c_files =				\
	tepl-buffer-input-stream.c		\
	tepl-close-confirm-dialog-single.c	\
	tepl-encoding-converter.c		\
	tepl-file-content.c			\
	tepl-file-content-loader.c		\
	tepl-io-error-info-bar.c		\
	tepl-progress-info-bar.c		\
	tepl-signal-group.c			\
	tepl-tab-saving.c

tepl_built_public_headers =		\
	tepl-enum-types.h

tepl_built_public_c_files =		\
	tepl-enum-types.c

# Helper Libtool library, so that the private functions can be used in unit
# tests.
noinst_LTLIBRARIES = libtepl-core.la

libtepl_core_la_SOURCES =		\
	$(tepl_private_c_files)		\
	$(tepl_private_headers)		\
	$(tepl_public_c_files)		\
	$(tepl_public_headers)

nodist_libtepl_core_la_SOURCES =	\
	$(tepl_built_public_c_files)	\
	$(tepl_built_public_headers)

BUILT_SOURCES =				\
	$(tepl_built_public_c_files)	\
	$(tepl_built_public_headers)

libtepl_core_la_CFLAGS =	\
	$(CODE_COVERAGE_CFLAGS)

libtepl_core_la_LDFLAGS =	\
	-no-undefined		\
	$(WARN_LDFLAGS)

libtepl_core_la_LIBADD =	\
	$(CODE_COVERAGE_LIBS)

# The real library.
lib_LTLIBRARIES = libtepl-@TEPL_API_VERSION@.la

libtepl_@TEPL_API_VERSION@_la_SOURCES =

libtepl_@TEPL_API_VERSION@_la_LIBADD =	\
	libtepl-core.la			\
	$(TEPL_DEP_LIBS)

libtepl_@TEPL_API_VERSION@_la_LDFLAGS =		\
	-version-info $(TEPL_LT_VERSION)	\
	-no-undefined				\
	-export-symbols-regex "^tepl_.*"	\
	$(WARN_LDFLAGS)

libtepl_includedir = $(includedir)/tepl-@TEPL_API_VERSION@/tepl
libtepl_include_HEADERS = $(tepl_public_headers)
nodist_libtepl_include_HEADERS = $(tepl_built_public_headers)

ENUM_TYPES = $(tepl_public_headers)

tepl-enum-types.h: tepl-enum-types.h.template $(ENUM_TYPES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template tepl-enum-types.h.template $(ENUM_TYPES)) > $@

tepl-enum-types.c: tepl-enum-types.c.template $(ENUM_TYPES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template tepl-enum-types.c.template $(ENUM_TYPES)) > $@

EXTRA_DIST =				\
	tepl-enum-types.c.template	\
	tepl-enum-types.h.template

CLEANFILES = $(BUILT_SOURCES)

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Tepl-@TEPL_API_VERSION@.gir

INTROSPECTION_SCANNER_ENV = CC="$(CC)"

Tepl-@TEPL_API_VERSION@.gir: libtepl-@TEPL_API_VERSION@.la

INTROSPECTION_SCANNER_ARGS =		\
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	--c-include=tepl/tepl.h		\
	--warn-all

Tepl_@TEPL_API_VERSION@_gir_NAMESPACE = Tepl
Tepl_@TEPL_API_VERSION@_gir_VERSION = @TEPL_API_VERSION@
Tepl_@TEPL_API_VERSION@_gir_CFLAGS = $(TEPL_DEP_CFLAGS)
Tepl_@TEPL_API_VERSION@_gir_SCANNERFLAGS = $(WARN_SCANNERFLAGS)
Tepl_@TEPL_API_VERSION@_gir_LIBS = libtepl-@TEPL_API_VERSION@.la
Tepl_@TEPL_API_VERSION@_gir_FILES =	\
	$(tepl_public_headers)		\
	$(tepl_public_c_files)		\
	$(tepl_built_public_headers)	\
	$(tepl_built_public_c_files)

Tepl_@TEPL_API_VERSION@_gir_INCLUDES = Gtk-3.0 GtkSource-4 Amtk-5
Tepl_@TEPL_API_VERSION@_gir_PACKAGES = gtk+-3.0 gtksourceview-4 amtk-5
Tepl_@TEPL_API_VERSION@_gir_EXPORT_PACKAGES = tepl-@TEPL_API_VERSION@

girdir = $(datadir)/gir-1.0
gir_DATA = Tepl-@TEPL_API_VERSION@.gir

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = Tepl-@TEPL_API_VERSION@.typelib

CLEANFILES +=		\
	$(gir_DATA)	\
	$(typelib_DATA)

endif # HAVE_INTROSPECTION

-include $(top_srcdir)/git.mk
