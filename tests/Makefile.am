AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(DEP_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

LDADD = $(top_builddir)/gtef/libgtef-core.la	\
	$(DEP_LIBS)

noinst_PROGRAMS = $(TEST_PROGS)
TEST_PROGS =

TEST_PROGS += test-fold-region
test_fold_region_SOURCES = test-fold-region.c

TEST_PROGS += test-gutter-renderer-folds
test_gutter_renderer_folds_SOURCES =		\
	gtef-gutter-renderer-folds-sub.c	\
	gtef-gutter-renderer-folds-sub.h	\
	test-gutter-renderer-folds.c

TEST_PROGS += test-menu
test_menu_SOURCES = test-menu.c

TEST_PROGS += test-tab
test_tab_SOURCES = test-tab.c

-include $(top_srcdir)/git.mk
