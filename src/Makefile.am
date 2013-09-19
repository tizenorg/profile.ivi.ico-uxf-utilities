lib_LTLIBRARIES = libico-util.la

libico_util_la_CFLAGS = -I../include $(OPT_CFLAGS)
libico_util_la_LIBADD =  $(OPT_LIBS) -lwebsockets
libico_util_la_LDFLAGS = -version-info 0:2:0 
libico_util_la_SOURCES =          \
	ico_uws.c	\
	ico_log.c

AM_CFLAGS = $(GCC_CFLAGS)
AM_CPPFLAGS = $(GCC_CFLAGS)	\
	-DDATADIR='"$(datadir)"'		\
	-DMODULEDIR='"$(moduledir)"'		\
	-DLIBEXECDIR='"$(libexecdir)"'
AM_LDFLAGS = -module -avoid-version -rpath $(libdir) -lwebsockets

.FORCE :

