AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(DEP_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

LDADD = $(top_builddir)/gtef/libgtef-core.la	\
	$(DEP_LIBS)

noinst_PROGRAMS = $(TEST_PROGS)
TEST_PROGS =

TEST_PROGS += test-tab
test_tab_SOURCES = test-tab.c

-include $(top_srcdir)/git.mk
