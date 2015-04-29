dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.59])
AC_INIT(BSP.Duang, 0.0.1, bugs@bsgroup.org, bspd, http://p.bsgroup.org/)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 subdir-objects])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([disable])
AC_PREFIX_DEFAULT([/opt/bspd])
AC_PROG_CC
AC_PROG_CXX([gcc c++ g++ cpp])
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_HEADER_STDC

test "$prefix" = NONE && prefix=/opt/bspd

AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_TYPE_SIZE_T
LT_INIT

AC_SEARCH_LIBS(gethugepagesizes, hugetlbfs)
AC_SEARCH_LIBS(lt_dlinit, ltdl, [], [AC_MSG_ERROR([libltdl needed])])

AC_CHECK_LIB(stdc++, main, [], [AC_MSG_ERROR([libstdc++ needed])])

CPPFLAGS="-I$PWD -I$PWD/deps/hiredis -I$PWD/deps/libmongo-client -I$PWD/lua -I$PWD/lz4 -I$PWD/mariadb-client -I$PWD/snappy -I$PWD/sqlite -I$PWD/zlib"

# libbsp
trylibbsp="no"
AC_ARG_WITH(
    [libbsp], 
    [AS_HELP_STRING([--with-libbsp=DIR], [Specify location of libbsp installation])], 
    [
        if test "x$withval" != "xno"; then
            trylibbsp=$withval
        fi
    ]
)
AC_CACHE_CHECK(
    [for libbsp installation], 
    ac_cv_libbsp_ver, 
    [
        LIBS=`pkg-config --libs bsp`" $LIBS"
        CPPFLAGS=`pkg-config --cflags bsp`" $CPPFLAGS"
        #Try compile
        AC_TRY_LINK(
            [#include <bsp.h>], 
            [int t = BSP_BOOTSTRAP_STANDARD;], 
            [lb_linked=yes], 
            [lb_linked=no]
        )
        if test $lg_linked = no; then
            AC_MSG_ERROR([libbsp not found])
        else
            ac_cv_libbsp_ver=`pkg-config --modversion bsp`
        fi
    ]
)

AC_DEFINE_UNQUOTED([BSPD_PREFIX_DIR], ["$prefix"], [Prefix directory of BSP application])
AC_DEFINE_UNQUOTED([BSPD_CONF_FILE], ["etc/bspd.runtime.conf"], [Configuration of BSP application])

AC_OUTPUT([
Makefile
deps/Makefile
deps/zlib/Makefile
deps/lz4/Makefile
deps/snappy/Makefile
deps/sqlite/Makefile
deps/mariadb-client/Makefile
deps/hiredis/Makefile
deps/lua/Makefile
src/Makefile
po/Makefile.in
])
