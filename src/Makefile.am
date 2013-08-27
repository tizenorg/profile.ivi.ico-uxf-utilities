lib_LTLIBRARIES = libico-util-com.la

libico_util_com_la_CFLAGS = -I../include $(OPT_CFLAGS)
libico_util_com_la_LIBADD =  $(OPT_LIBS) -lwebsockets
libico_util_com_la_LDFLAGS = -version-info 0:2:0 
libico_util_com_la_SOURCES =          \
	ico_uws.c

AM_CFLAGS = $(GCC_CFLAGS)
AM_CPPFLAGS = $(GCC_CFLAGS)	\
	-DDATADIR='"$(datadir)"'		\
	-DMODULEDIR='"$(moduledir)"'		\
	-DLIBEXECDIR='"$(libexecdir)"'
AM_LDFLAGS = -module -avoid-version -rpath $(libdir) -lwebsockets

.FORCE :

