AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(AMTK_DEP_CFLAGS)	\
	$(TEPL_DEP_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

# The order is important here: link Amtk after Tepl, since Tepl uses Amtk.
# This is how Unix linkers work by default, see for example the documentation
# of the --library option in the ld manpage.
LDADD = $(top_builddir)/tepl/libtepl-core.la	\
	$(top_builddir)/amtk/libamtk-core.la	\
	$(AMTK_DEP_LIBS)			\
	$(TEPL_DEP_LIBS)

noinst_PROGRAMS = $(TEST_PROGS)
TEST_PROGS =

TEST_PROGS += test-fold-region
test_fold_region_SOURCES = test-fold-region.c

TEST_PROGS += test-gutter-renderer-folds
test_gutter_renderer_folds_SOURCES =		\
	tepl-gutter-renderer-folds-sub.c	\
	tepl-gutter-renderer-folds-sub.h	\
	test-gutter-renderer-folds.c

TEST_PROGS += test-menu
test_menu_SOURCES = test-menu.c

TEST_PROGS += test-tab
test_tab_SOURCES = test-tab.c

-include $(top_srcdir)/git.mk
