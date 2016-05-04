## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6

# The name of the module.
# For library-web, it's simpler if the DOC_MODULE follows the scheme:
# $name-$major.$minor
# (to show the API online at developer.gnome.org)
# So, add ".0".
DOC_MODULE = gtef-@GTEF_API_VERSION@.0

# The top-level XML file (SGML in the past).
DOC_MAIN_SGML_FILE = gtef-docs.xml

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS = --rebuild-types

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR = $(top_srcdir)/gtef

# Used for dependencies
HFILE_GLOB = $(top_srcdir)/gtef/*.h
CFILE_GLOB = $(top_srcdir)/gtef/*.c

# Header files or dirs to ignore when scanning. Use base file/dir names
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h private_code
IGNORE_HFILES =

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = --xml-mode --output-format=xml

# Images to copy into HTML directory
HTML_IMAGES =

# Extra XML files that are included by $(DOC_MAIN_SGML_FILE)
content_files =

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS =							\
	--extra-dir=$(GLIB_PREFIX)/share/gtk-doc/html/gobject		\
	--extra-dir=$(GLIB_PREFIX)/share/gtk-doc/html/glib		\
	--extra-dir=$(GTK_PREFIX)/share/gtk-doc/html/gtk3

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
GTKDOC_CFLAGS = -I$(top_srcdir) $(DEP_CFLAGS)
GTKDOC_LIBS =							\
	$(top_builddir)/gtef/libgtef-@GTEF_API_VERSION@.la	\
	$(DEP_LIBS)

MAINTAINERCLEANFILES =			\
	$(DOC_MODULE)-overrides.txt	\
	$(DOC_MODULE).types

# include common portion ...
include $(top_srcdir)/gtk-doc.make

# gtk-doc.make has already a recipe for the $(DOC_MODULE)-sections.txt target.
# So override it here.
$(builddir)/$(DOC_MODULE)-sections.txt: Makefile $(srcdir)/gtef-sections.txt.in
	$(AM_V_GEN) cp $(srcdir)/gtef-sections.txt.in $@

EXTRA_DIST +=			\
	gtef-docs.xml.in	\
	gtef-sections.txt.in

DISTCLEANFILES =					\
	gtef-docs.xml					\
	gtef-@GTEF_API_VERSION@.0-sections.txt

-include $(top_srcdir)/git.mk
