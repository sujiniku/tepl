include $(top_srcdir)/aminclude_static.am
clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean

@VALGRIND_CHECK_RULES@

# https://github.com/dtrebbien/GNOME.supp
VALGRIND_SUPPRESSIONS_FILES =					\
	$(GLIB_PREFIX)/share/glib-2.0/valgrind/glib.supp	\
	$(top_srcdir)/../GNOME.supp/build/base.supp		\
	$(top_srcdir)/../GNOME.supp/build/pango.supp		\
	$(top_srcdir)/../GNOME.supp/build/gdk.supp		\
	$(top_srcdir)/../GNOME.supp/build/gtk3.supp

AM_CPPFLAGS = 			\
	-I$(top_srcdir) 	\
	$(WARN_CFLAGS) 		\
	$(TEPL_DEP_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

LDADD = $(top_builddir)/tepl/libtepl-core.la	\
	$(TEPL_DEP_LIBS)

noinst_PROGRAMS = $(UNIT_TEST_PROGS)
TESTS = $(UNIT_TEST_PROGS)
UNIT_TEST_PROGS =

UNIT_TEST_PROGS += test-buffer-input-stream
test_buffer_input_stream_SOURCES = test-buffer-input-stream.c

UNIT_TEST_PROGS += test-encoding
test_encoding_SOURCES = test-encoding.c

UNIT_TEST_PROGS += test-encoding-converter
test_encoding_converter_SOURCES = test-encoding-converter.c

UNIT_TEST_PROGS += test-file
test_file_SOURCES = test-file.c

UNIT_TEST_PROGS += test-file-content
test_file_content_SOURCES = test-file-content.c

UNIT_TEST_PROGS += test-file-loader
test_file_loader_SOURCES = test-file-loader.c

UNIT_TEST_PROGS += test-file-metadata
test_file_metadata_SOURCES = test-file-metadata.c

UNIT_TEST_PROGS += test-file-saver
test_file_saver_SOURCES = test-file-saver.c

UNIT_TEST_PROGS += test-fold-region
test_fold_region_SOURCES = test-fold-region.c

UNIT_TEST_PROGS += test-info-bar
test_info_bar_SOURCES = test-info-bar.c

UNIT_TEST_PROGS += test-metadata-store
test_metadata_store_SOURCES = test-metadata-store.c

UNIT_TEST_PROGS += test-notebook
test_notebook_SOURCES = test-notebook.c

UNIT_TEST_PROGS += test-utils
test_utils_SOURCES = test-utils.c

if INSTALLED_TESTS
insttestdir = $(libexecdir)/installed-tests/$(PACKAGE)-@TEPL_API_VERSION@
insttest_PROGRAMS = $(UNIT_TEST_PROGS)

%.test: %$(EXEEXT) Makefile
	$(AM_V_GEN) (echo '[Test]' > $@.tmp; \
	echo 'Description=Tepl test' >> $@.tmp; \
	echo 'Exec=$(insttestdir)/$<' >> $@.tmp; \
	echo 'Type=session' >> $@.tmp; \
	echo 'Output=TAP' >> $@.tmp; \
	mv $@.tmp $@)

test_files = $(UNIT_TEST_PROGS:=.test)

CLEANFILES = $(test_files)

testmetadir = $(datadir)/installed-tests/$(PACKAGE)-@TEPL_API_VERSION@
testmeta_DATA = $(test_files)

endif # INSTALLED_TESTS

-include $(top_srcdir)/git.mk
