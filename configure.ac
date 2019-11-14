#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([trillian], [1.0], [akshay42@protonmail.com])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday])

AC_CONFIG_FILES([
 Makefile
 src/Makefile
])

AC_OUTPUT
