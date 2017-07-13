## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6

# The name of the module.
# For library-web, it's simpler if the DOC_MODULE follows the scheme:
# $name-$major.$minor
# (to show the API online at developer.gnome.org)
# So, add ".0".
DOC_MODULE = tepl-@TEPL_API_VERSION@.0

# The top-level XML file (SGML in the past).
DOC_MAIN_SGML_FILE = tepl-docs.xml

# Extra options to supply to gtkdoc-scan
SCAN_OPTIONS = --rebuild-types

# The directory containing the source code. Relative to $(srcdir)
DOC_SOURCE_DIR = $(top_srcdir)/tepl $(top_srcdir)/amtk

# Used for dependencies
HFILE_GLOB = $(top_srcdir)/tepl/*.h $(top_srcdir)/amtk/*.h
CFILE_GLOB = $(top_srcdir)/tepl/*.c $(top_srcdir)/amtk/*.c

# Header files or dirs to ignore when scanning. Use base file/dir names
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h private_code
IGNORE_HFILES =					\
	amtk.h					\
	amtk-utils.h				\
	tepl.h					\
	tepl-application-window-actions.h	\
	tepl-buffer-input-stream.h		\
	tepl-encoding-converter.h		\
	tepl-encoding-private.h			\
	tepl-file-content-loader.h		\
	tepl-io-error-info-bar.h		\
	tepl-progress-info-bar.h

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS = --xml-mode --output-format=xml

# Images to copy into HTML directory
HTML_IMAGES =

# Extra XML files that are included by $(DOC_MAIN_SGML_FILE)
content_files =			\
	api-breaks.xml		\
	intro.xml

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS =

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
GTKDOC_CFLAGS = -I$(top_srcdir) $(DEP_CFLAGS)
GTKDOC_LIBS =							\
	$(top_builddir)/tepl/libtepl-@TEPL_API_VERSION@.la	\
	$(DEP_LIBS)

MAINTAINERCLEANFILES =			\
	$(DOC_MODULE)-overrides.txt	\
	$(DOC_MODULE).types

# include common portion ...
include $(top_srcdir)/gtk-doc.make

EXTRA_DIST +=			\
	tepl-docs.xml.in	\
	intro.xml.in

DISTCLEANFILES =		\
	tepl-docs.xml		\
	intro.xml

-include $(top_srcdir)/git.mk
