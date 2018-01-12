#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(midiguitar, 0.1, stazed10@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([main.cpp])
AC_CONFIG_HEADERS([config.h])

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
fi

AC_CHECK_LIB([jack],[main], [ac_jack="yes"], [ac_jack="no"])
if test "x$ac_jack" = "xno"; then
AC_MSG_ERROR([JACK library not found.])
else 
   LIBS="$LIBS -ljack"
fi

AC_CHECK_LIB([pthread], [main], [ac_pthread="yes"], [ac_pthread="no"] )
if test "x$ac_pthread" = "xno"; then
AC_MSG_ERROR([pthread library not found.])
else 
   LIBS="$LIBS -lpthread"
fi

AC_ARG_ENABLE(rtmidi,
    [AS_HELP_STRING(--disable-rtmidi, [Disable RtMidi build. RtMidi is the default build])],
    [rtmidi=$enableval],
    [rtmidi=yes])

if test "$rtmidi" != "no"; then
    AC_DEFINE(RTMIDI_SUPPORT, 1, [Define to 1 to enable RtMidi build])
    if test "x$ac_asound" = "xyes"; then
    AC_DEFINE([__LINUX_ALSA__], 1 , [Define to 1 to enable alsa to be used by RtMidi])
    fi
    if test "x$ac_jack" = "xyes"; then
    AC_DEFINE([__UNIX_JACK__], 1 , [Define to 1 to enable jack to be used by RtMidi])
    fi
    AC_MSG_RESULT([RtMidi native build enabled]);
else
    AC_MSG_WARN([RtMidi native build disabled]);
fi

if test "x$ac_asound" = "xyes"; then
AC_ARG_ENABLE(alsa,
    [AS_HELP_STRING(--enable-alsa, [Enable legacy alsa support])],
    [alsa=$enableval],
    [alsa=no])
fi

if test "$alsa" != "no"; then
    AC_DEFINE(ALSA_SUPPORT, 1, [Define to 1 to enable alsa legacy support])
    AC_MSG_RESULT([alsa legacy support enabled]);
else
    AC_MSG_WARN([alsa legacy support disabled]);
fi

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([pow])

AC_OUTPUT(Makefile)
