@CODE_COVERAGE_RULES@

AM_CPPFLAGS =				\
	-DG_LOG_DOMAIN=\"Amtk\"		\
	-DAMTK_COMPILATION		\
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	$(WARN_CFLAGS)			\
	$(CODE_COVERAGE_CPPFLAGS)	\
	$(AMTK_DEP_CFLAGS)

amtk_public_headers =				\
	amtk.h					\
	amtk-action-info.h			\
	amtk-action-info-central-store.h	\
	amtk-action-info-store.h		\
	amtk-action-map.h			\
	amtk-application-window.h		\
	amtk-factory.h				\
	amtk-menu-factory.h			\
	amtk-menu-item.h			\
	amtk-menu-shell.h			\
	amtk-types.h				\
	amtk-utils.h

amtk_public_c_files =				\
	amtk-action-info.c			\
	amtk-action-info-central-store.c	\
	amtk-action-info-store.c		\
	amtk-action-map.c			\
	amtk-application-window.c		\
	amtk-factory.c				\
	amtk-menu-factory.c			\
	amtk-menu-item.c			\
	amtk-menu-shell.c			\
	amtk-utils.c

# Helper Libtool library, so that the private functions can be used in unit
# tests.
noinst_LTLIBRARIES = libamtk-core.la

libamtk_core_la_SOURCES =	\
	$(amtk_public_c_files)	\
	$(amtk_public_headers)

libamtk_core_la_CFLAGS =	\
	$(CODE_COVERAGE_CFLAGS)

libamtk_core_la_LDFLAGS =	\
	-no-undefined		\
	$(WARN_LDFLAGS)

libamtk_core_la_LIBADD =	\
	$(CODE_COVERAGE_LIBS)

# The real library.
lib_LTLIBRARIES = libamtk-@TEPL_API_VERSION@.la

libamtk_@TEPL_API_VERSION@_la_SOURCES =

libamtk_@TEPL_API_VERSION@_la_LIBADD =	\
	libamtk-core.la			\
	$(AMTK_DEP_LIBS)

libamtk_@TEPL_API_VERSION@_la_LDFLAGS =		\
	-version-info $(AMTK_LT_VERSION)	\
	-no-undefined				\
	-export-symbols-regex "^amtk_.*"	\
	$(WARN_LDFLAGS)

libamtk_includedir = $(includedir)/amtk-@TEPL_API_VERSION@/amtk
libamtk_include_HEADERS = $(amtk_public_headers)

CLEANFILES =

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Amtk-@TEPL_API_VERSION@.gir

INTROSPECTION_SCANNER_ENV = CC="$(CC)"

Amtk-@TEPL_API_VERSION@.gir: libamtk-@TEPL_API_VERSION@.la

INTROSPECTION_SCANNER_ARGS =		\
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	--c-include=amtk/amtk.h		\
	--warn-all

Amtk_@TEPL_API_VERSION@_gir_NAMESPACE = Amtk
Amtk_@TEPL_API_VERSION@_gir_VERSION = @TEPL_API_VERSION@
Amtk_@TEPL_API_VERSION@_gir_CFLAGS = $(AMTK_DEP_CFLAGS)
Amtk_@TEPL_API_VERSION@_gir_SCANNERFLAGS = $(WARN_SCANNERFLAGS)
Amtk_@TEPL_API_VERSION@_gir_LIBS = libamtk-@TEPL_API_VERSION@.la
Amtk_@TEPL_API_VERSION@_gir_FILES =	\
	$(amtk_public_headers)		\
	$(amtk_public_c_files)

Amtk_@TEPL_API_VERSION@_gir_INCLUDES = Gtk-3.0
Amtk_@TEPL_API_VERSION@_gir_PACKAGES = gtk+-3.0
Amtk_@TEPL_API_VERSION@_gir_EXPORT_PACKAGES = amtk-@TEPL_API_VERSION@

girdir = $(datadir)/gir-1.0
gir_DATA = Amtk-@TEPL_API_VERSION@.gir

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = Amtk-@TEPL_API_VERSION@.typelib

CLEANFILES +=		\
	$(gir_DATA)	\
	$(typelib_DATA)

endif # HAVE_INTROSPECTION

-include $(top_srcdir)/git.mk
