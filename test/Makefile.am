TESTS_ENVIRONMENT = $(SHELL) $(top_srcdir)/test/run_test.sh

export abs_builddir

AM_CFLAGS = $(GCC_CFLAGS)
AM_CPPFLAGS = $(GCC_CFLAGS)

noinst_PROGRAMS =		\
	tst_ico_uws_client	\
	tst_ico_uws_server	\
	tst_ico_uws_multi_client \
	tst_ico_uws_multi_server

check_LTLIBRARIES = $(TESTS)
check_PROGRAMS = tst_ico_uws_client tst_ico_uws_server tst_ico_uws_multi_client tst_ico_uws_multi_server

test_common_lib = -lwebsockets
test_target_lib = ../src/.libs/libico-util-com.so

tst_ico_uws_client_SOURCES = tst_ico_uws_client.c
tst_ico_uws_client_CFLAGS = -I../include $(OPT_CFLAGS)
tst_ico_uws_client_LDADD = $(test_target_lib) $(OPT_LIBS) $(test_common_lib)

tst_ico_uws_server_SOURCES = tst_ico_uws_server.c
tst_ico_uws_server_CFLAGS = -I../include $(OPT_CFLAGS)
tst_ico_uws_server_LDADD = $(test_target_lib) $(OPT_LIBS) $(test_common_lib)

tst_ico_uws_multi_client_SOURCES = tst_ico_uws_multi_client.c
tst_ico_uws_multi_client_CFLAGS = -I../include $(OPT_CFLAGS)
tst_ico_uws_multi_client_LDADD = $(test_target_lib) $(OPT_LIBS) $(test_common_lib)

tst_ico_uws_multi_server_SOURCES = tst_ico_uws_multi_server.c
tst_ico_uws_multi_server_CFLAGS = -I../include $(OPT_CFLAGS)
tst_ico_uws_multi_server_LDADD = $(test_target_lib) $(OPT_LIBS) $(test_common_lib)

EXTRA_DIST = run_test.sh

