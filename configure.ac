AC_INIT([xings-user-accounts], [0.2.4],
	[https://github.com/matiasdelellis/xings-user-accounts])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz tar-ustar])
AM_MAINTAINER_MODE([enable])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_HEADER_STDC

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT

# Internationalization support

IT_PROG_INTLTOOL([0.40.1])

GETTEXT_PACKAGE=xings-user-accounts
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

AC_PATH_XTRA

AC_PATH_PROG([GLIB_MKENUMS],[glib-mkenums])

AC_CHECK_LIB(m, floor)

dnl ==============================================
dnl Check that we meet the dependencies
dnl ==============================================

GLIB_REQUIRED_VERSION=2.58.3
GTK_REQUIRED_VERSION=3.24.5
GDKPIXBUF_REQUIRED_VERSION=2.38.1
POLKIT_REQUIRED_VERSION=0.105
ACCOUNTSSERVICE_REQUIRED_VERSION=0.6.45
PWQUALITY_REQUIRED_VERSION=1.4.0
KRB5_REQUIRED_VERSION=1.17

PKG_CHECK_MODULES(XINGS_USER_ACCOUNTS,
                  glib-2.0 >= $GLIB_REQUIRED_VERSION
                  gtk+-3.0 >= $GTK_REQUIRED_VERSION
                  gdk-pixbuf-2.0 >= $GDKPIXBUF_REQUIRED_VERSION
                  polkit-gobject-1 >= $POLKIT_REQUIRED_VERSION
                  accountsservice >= $ACCOUNTSSERVICE_REQUIRED_VERSION
                  pwquality >= $PWQUALITY_REQUIRED_VERSION
                  krb5 >= $KRB5_REQUIRED_VERSION
                  gthread-2.0
                  gio-2.0
                  gio-unix-2.0)

dnl ============================================
dnl Optional dependency to take face from webcam
dnl ============================================

AC_ARG_WITH([cheese],
            AS_HELP_STRING([--with-cheese], [enable cheese webcam support]),,
            with_cheese=auto)

if test x"$with_cheese" != x"no" ; then
	PKG_CHECK_MODULES(CHEESE, cheese-gtk >= 3.5.91 cheese, [have_cheese=yes], [have_cheese=no])
	if test x${have_cheese} = xyes; then
		AC_DEFINE(HAVE_CHEESE, 1, [Define to 1 to enable cheese webcam support])
	fi
	if test x${with_cheese} = xyes && test x${have_cheese} = xno; then
		AC_MSG_ERROR([Cheese configured but not found])
	fi
else
	have_cheese=no
fi
AM_CONDITIONAL(BUILD_CHEESE, test x${have_cheese} = xyes)

dnl =======================================
dnl Update Mime Database
dnl =======================================

AC_PATH_PROG(UPDATE_MIME_DATABASE, update-mime-database, no)

AC_ARG_ENABLE(update-mimedb,
              AS_HELP_STRING([--disable-update-mimedb],
                             [do not update mime database after installation]),,
                             enable_update_mimedb=yes)
AM_CONDITIONAL(ENABLE_UPDATE_MIMEDB, test x$enable_update_mimedb = xyes)

dnl =======================================
dnl Finish
dnl =======================================

# Turn on the additional warnings last

AC_ARG_ENABLE(more-warnings,
              AS_HELP_STRING([--enable-more-warnings],
                             [Maximum compiler warnings]),
              set_more_warnings="$enableval",[
              if test -d $srcdir/.git; then
                      set_more_warnings=yes
              else
                      set_more_warnings=no
              fi])

AC_MSG_CHECKING(for more warnings)
if test "$GCC" = "yes" -a "$set_more_warnings" != "no"; then
	AC_MSG_RESULT(yes)
	CFLAGS="\
	-Wall -Wclobbered -Wempty-body -Wignored-qualifiers \
	-Wmissing-field-initializers -Wmissing-parameter-type \
	-Wold-style-declaration -Woverride-init -Wtype-limits \
	-Wuninitialized \
	-Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes \
	-Wnested-externs -Wpointer-arith \
	-Wcast-align -Wsign-compare \
	$CFLAGS"

	# Only add this when optimizing is enabled (default)
	AC_MSG_CHECKING([whether optimization is enabled])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if __OPTIMIZE__ == 0
	                                     #error No optimization
	                                     #endif
	                                   ]], [[]])],
	                  [has_optimization=yes],
	                  [has_optimization=no])
	if test $has_optimization = yes; then
		CFLAGS="$CFLAGS -Wp,-D_FORTIFY_SOURCE=2"
	fi
	AC_MSG_RESULT($has_optimization)

	for option in -Wno-strict-aliasing -Wno-sign-compare; do
		SAVE_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $option"
		AC_MSG_CHECKING([whether gcc understands $option])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
		                  [has_option=yes],
		                  [has_option=no])
		if test $has_option = no; then
		        CFLAGS="$SAVE_CFLAGS"
		fi
		AC_MSG_RESULT($has_option)
		unset has_option
		unset SAVE_CFLAGS
	done
	unset option
else
	AC_MSG_RESULT(no)
fi

dnl ---------------------------------------------------------------------------
dnl - Is docbook2man available?
dnl ---------------------------------------------------------------------------
AC_PATH_PROG(DOCBOOK2MAN, docbook2man, no)
if test "$DOCBOOK2MAN" = "no" ; then
    AC_MSG_WARN([docbook2man not found, will not be able to build man documentation])
fi
AM_CONDITIONAL(HAVE_DOCBOOK2MAN, [test "$DOCBOOK2MAN" != "no"])

dnl ---------------------------------------------------------------------------
dnl - Enable gsettings schema macros
dnl ---------------------------------------------------------------------------
GLIB_GSETTINGS

dnl ---------------------------------------------------------------------------
dnl - Makefiles, etc.
dnl --------------------------------------------------------------------------
AC_CONFIG_FILES([
	Makefile
	data/Makefile
	data/appdata/Makefile
	data/faces/Makefile
	data/icons/Makefile
	man/Makefile
	po/Makefile.in
	src/Makefile
])

AC_OUTPUT

dnl ==========================================================================
echo "
                    xings-user-accounts v$VERSION
                  ==============================

        prefix:                    ${prefix}
        compiler:                  ${CC}
        cflags:                    ${CFLAGS}
        cppflags:                  ${CPPFLAGS}

        Webcam support:            ${have_cheese}
"
