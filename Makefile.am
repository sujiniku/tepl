SUBDIRS = po amtk tepl tests testsuite docs

@CODE_COVERAGE_RULES@

AM_DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-introspection

pkgconfig_DATA = tepl-$(TEPL_API_VERSION).pc
DISTCLEANFILES = $(pkgconfig_DATA)

EXTRA_DIST =		\
	tepl.doap	\
	tepl.pc.in	\
	HACKING

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	gtk-doc.make \
	INSTALL

-include $(top_srcdir)/git.mk
