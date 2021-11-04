dnl config.m4 for extension performance

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use 'with':

 PHP_ARG_WITH([performance],
   [for performance support],
   [AS_HELP_STRING([--with-performance],
     [Include performance support])])



if test "$PHP_PERFORMANCE" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, PERFORMANCE_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-performance -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/performance.h"  # you most likely want to change this
  dnl if test -r $PHP_PERFORMANCE/$SEARCH_FOR; then # path given as parameter
  dnl   PERFORMANCE_DIR=$PHP_PERFORMANCE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for performance files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PERFORMANCE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PERFORMANCE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the performance distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-performance -> add include path
  dnl PHP_ADD_INCLUDE($PERFORMANCE_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-performance -> check for lib and symbol presence
  dnl LIBNAME=PERFORMANCE # you may want to change this
  dnl LIBSYMBOL=PERFORMANCE # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_PERFORMANCE_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your performance library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PERFORMANCE_DIR/$PHP_LIBDIR, PERFORMANCE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PERFORMANCE_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your performance library.])
  dnl ],[
  dnl   -L$PERFORMANCE_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PERFORMANCE_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_PERFORMANCE, 1, [ Have performance support ])
PHP_PERFORMANCE_SOURCE_FILES="\
    performance.c \
    src/performance_util.c"
  PHP_NEW_EXTENSION(performance, $PHP_PERFORMANCE_SOURCE_FILES, $ext_shared)

fi
