#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(midiguitar, 0.1, stazed10@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([main.cpp])
# AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([fltk], [main], [ac_fltk="yes"], [ac_fltk="no"])
if test "x$ac_fltk" = "xno"; then
AC_MSG_ERROR([unable to find fltk library.])
else
   LIBS="$LIBS -lfltk"
fi

AC_CHECK_LIB([asound], [snd_pcm_open], [ac_asound="yes"], [ac_asound="no"])
if test "x$ac_asound" = "xno"; then
AC_MSG_ERROR([ALSA libasound library not found.])
else 
   LIBS="$LIBS -lasound"
   AC_DEFINE(__LINUX_ALSA__)
fi

AC_CHECK_LIB([jack],[main], [ac_jack="yes"], [ac_jack="no"])
if test "x$ac_jack" = "xno"; then
AC_MSG_ERROR([JACK library not found.])
else 
   LIBS="$LIBS -ljack"
   AC_DEFINE(__UNIX_JACK__ )
fi

AC_CHECK_LIB([pthread], [main], [ac_pthread="yes"], [ac_pthread="no"] )
if test "x$ac_pthread" = "xno"; then
AC_MSG_ERROR([pthread library not found.])
else 
   LIBS="$LIBS -lpthread"
fi

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([pow])

AC_OUTPUT(Makefile)
