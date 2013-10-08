AC_PREREQ([2.68])
AC_INIT([ico-uxf-utilities],
        [0.9.01],
        [https://BUG-REPORT-ADDRESS])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz])

AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_CC

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT([disable-static])

PKG_PROG_PKG_CONFIG()

PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([DLOG], [dlog])
OPT_CFLAGS="$GLIB_CFLAGS $DLOG_CFLAGS"
OPT_LIBS="$GLIB_LIBS $DLOG_LIBS"
AC_SUBST(OPT_CFLAGS)
AC_SUBST(OPT_LIBS)

AC_CHECK_HEADERS([execinfo.h])

AC_CHECK_FUNCS([mkostemp strchrnul])

if test "x$GCC" = "xyes"; then
	my_common_gcc_flags="-Wall -Wextra -Wno-unused-parameter \
		-Wno-missing-field-initializers -g -fvisibility=hidden"
	GCC_CFLAGS="$my_common_gcc_flags \
		-Wstrict-prototypes -Wmissing-prototypes"
	GCC_CXXFLAGS="$my_common_gcc_flags"
fi
AC_SUBST(GCC_CFLAGS)
AC_SUBST(GCC_CXXFLAGS)

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 test/Makefile])
AC_OUTPUT
