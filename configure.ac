#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(midiguitar, 1.0, stazed10@gmail.com)
#AM_INIT_AUTOMAKE(midiguitar, 1.0)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([main.cpp])
# AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([fltk], [main], [], [
  AC_MSG_ERROR([unable to find fltk library.])
])

AC_CHECK_LIB(asound, snd_pcm_open,,
     AC_MSG_ERROR(Essential library asound not found))

# FIXME: Replace `main' with a function in `-ljack':
AC_CHECK_LIB([jack], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([pow])

# FIXME: set up conditionals for these
AC_DEFINE(__LINUX_ALSA__)
AC_DEFINE(__UNIX_JACK__ )
AC_OUTPUT(Makefile)
