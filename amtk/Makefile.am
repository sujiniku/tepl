@CODE_COVERAGE_RULES@

AM_CPPFLAGS =				\
	-DG_LOG_DOMAIN=\"Amtk\"		\
	-DAMTK_COMPILATION		\
	-I$(top_builddir)		\
	-I$(top_srcdir)			\
	$(WARN_CFLAGS)			\
	$(CODE_COVERAGE_CPPFLAGS)	\
	$(DEP_CFLAGS)

amtk_public_headers =				\
	amtk.h					\
	amtk-action-info.h			\
	amtk-action-info-central-store.h	\
	amtk-action-info-store.h		\
	amtk-action-map.h			\
	amtk-menu-item.h			\
	amtk-menu-shell.h			\
	amtk-types.h

amtk_public_c_files =				\
	amtk-action-info.c			\
	amtk-action-info-central-store.c	\
	amtk-action-info-store.c		\
	amtk-action-map.c			\
	amtk-menu-item.c			\
	amtk-menu-shell.c

noinst_LTLIBRARIES = libamtk.la

libamtk_la_SOURCES =		\
	$(amtk_public_c_files)	\
	$(amtk_public_headers)

libamtk_la_CFLAGS =		\
	$(CODE_COVERAGE_CFLAGS)

libamtk_la_LDFLAGS =		\
	-no-undefined		\
	$(WARN_LDFLAGS)

libamtk_la_LIBADD =		\
	$(CODE_COVERAGE_LIBS)

libamtk_includedir = $(includedir)/tepl-@TEPL_API_VERSION@/amtk
libamtk_include_HEADERS = $(amtk_public_headers)

CLEANFILES =

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Amtk-@TEPL_API_VERSION@.gir

INTROSPECTION_SCANNER_ENV = CC="$(CC)"

Amtk-@TEPL_API_VERSION@.gir: libamtk.la
INTROSPECTION_SCANNER_ARGS =		\
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	--c-include=amtk/amtk.h		\
	--warn-all
Amtk_@TEPL_API_VERSION@_gir_NAMESPACE = Amtk
Amtk_@TEPL_API_VERSION@_gir_VERSION = @TEPL_API_VERSION@
Amtk_@TEPL_API_VERSION@_gir_CFLAGS = $(DEP_CFLAGS)
Amtk_@TEPL_API_VERSION@_gir_SCANNERFLAGS = $(WARN_SCANNERFLAGS)
Amtk_@TEPL_API_VERSION@_gir_LIBS = libamtk.la
Amtk_@TEPL_API_VERSION@_gir_FILES =	\
	$(amtk_public_headers)		\
	$(amtk_public_c_files)

Amtk_@TEPL_API_VERSION@_gir_INCLUDES = Gtk-3.0
Amtk_@TEPL_API_VERSION@_gir_PACKAGES = gtk+-3.0

girdir = $(datadir)/gir-1.0
gir_DATA = Amtk-@TEPL_API_VERSION@.gir

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = Amtk-@TEPL_API_VERSION@.typelib

CLEANFILES +=		\
	$(gir_DATA)	\
	$(typelib_DATA)

endif # HAVE_INTROSPECTION

if ENABLE_VAPIGEN
-include $(VAPIGEN_MAKEFILE)

amtk-@TEPL_API_VERSION@.vapi: Amtk-@TEPL_API_VERSION@.gir

VAPIGEN_VAPIS = amtk-@TEPL_API_VERSION@.vapi

amtk_@TEPL_API_VERSION@_vapi_DEPS = gtk+-3.0

amtk-@TEPL_API_VERSION@.deps:
	$(AM_V_GEN) for pkg in $(amtk_@TEPL_API_VERSION@_vapi_DEPS); do \
		echo $$pkg >> $@; \
	done

amtk_@TEPL_API_VERSION@_vapi_METADATADIRS = $(srcdir)
amtk_@TEPL_API_VERSION@_vapi_FILES = Amtk-@TEPL_API_VERSION@.gir

vapidir = $(datadir)/vala/vapi
vapi_DATA =				\
	amtk-@TEPL_API_VERSION@.vapi	\
	amtk-@TEPL_API_VERSION@.deps

CLEANFILES += $(vapi_DATA)

endif # ENABLE_VAPIGEN

-include $(top_srcdir)/git.mk
