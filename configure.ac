dnl ner: configure.ac

AC_PREREQ([2.59])

AC_INIT([ner], [0.0.1], [mforney@mforney.org])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_LANG([C++])

AM_PROG_AR
LT_INIT

AC_CONFIG_SRCDIR([src/ner.cc])
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AX_CXX_COMPILE_STDCXX_11
PKG_PROG_PKG_CONFIG([0.9.0])

dnl Check for libraries {{{

# GLib
PKG_CHECK_MODULES([glib], [glib-2.0])

# GMime
PKG_CHECK_MODULES([gmime], [gmime-2.6],,
    [PKG_CHECK_MODULES([gmime], [gmime-2.4])])

# Yaml-CPP
PKG_CHECK_MODULES([yaml_cpp], [yaml-cpp >= 0.5.0])

# NCurses
AC_CHECK_LIB(ncursesw, initscr, [AC_SUBST([ncurses_LIBS], ["-lncursesw"])],
    [AC_MSG_ERROR([ner requires ncursesw])])
AC_CHECK_HEADERS(ncursesw/ncurses.h,,
    [AC_CHECK_HEADERS(ncurses/ncurses.h,,
        [AC_CHECK_HEADERS(ncurses.h)])])

# Notmuch
AC_CHECK_LIB(notmuch, notmuch_database_open, [AC_SUBST([notmuch_LIBS],
    ["-lnotmuch"])], [AC_MSG_ERROR([ner requires libnotmuch])])

dnl }}}

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile notmuch/Makefile])
AC_OUTPUT

dnl vim: fdm=marker et sw=4 sts=4 ts=8

