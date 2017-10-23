#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([monfs], [0.1.1], [hitoshi.sato@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([include/monfs_config.h.in])
AC_CONFIG_HEADER([include/monfs_config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# --with-fuse
AC_MSG_CHECKING([fuse])
AC_ARG_WITH([fuse],
	AC_HELP_STRING([--with-fuse=PATH], [path to fuse [[/usr]]]),
	[fuse_prefix=$withval],
	[fuse_prefix=no])
if test x"$fuse_prefix" != xno; then
   AC_MSG_RESULT([$fuse_prefix])
   CPPFLAG="$CPPFLAGS -I$fuse_prefix/include"
   LDFLAGS="$LDFLAGS -R$fuse_prefix/lib -L$fuse_prefix/lib"
else
   AC_MSG_RESULT([/usr])
fi

# --with-libyaml
# AC_MSG_CHECKING([libyaml])
# AC_ARG_WITH([libyaml],
#	AC_HELP_STRING([--with-libyaml=PATH], [path to libyaml [[/usr]]]),
#	[libyaml_prefix=$withval],
#	[libyaml_prefix=no])
#
#if test x"$libyaml_prefix" != xno; then
#   AC_MSG_RESULT([$libyaml])
#   CPPFLAGS="$CPPFLAGS -I$libyaml_prefix/include"
#   LDFLAGS="$LDFLAGS -R$libyaml_prefix/lib -L$libyaml_prefix/lib"
#else
#   AC_MSG_RESULT([/usr])
#fi	      					      
						      
#  --with-sqlite3
AC_MSG_CHECKING([sqlite3])
AC_ARG_WITH([sqlite3],
	AC_HELP_STRING([--with-sqlite3=PATH], [path to sqlite3 [[/usr]]]),
	[sqlite3_prefix=$withval],
	[sqlite3_prefix=no])

if test x"$sqlite3_prefix" != xno; then
   AC_MSG_RESULT([$sqlite3_prefix])
   CPPFLAGS="$CPPFLAGS -I$sqlite3_prefix/include "
   LDFLAGS="$LDFLAGS -R$sqlite3_prefix/lib -L$sqlite3_prefix/lib"
else
   AC_MSG_RESULT([/usr])
fi		

# Checks for libraries.
AC_PROG_LIBTOOL
AC_CHECK_LIB([fuse], [fuse_main],, [AC_MSG_ERROR([libfuse not found])])
#AC_CHECK_LIB([yaml], [yaml_parser_initialize],, [AC_MSG_ERROR([libyaml not found])])
AC_CHECK_LIB([sqlite3], [sqlite3_open],, [AC_MSG_ERROR([libsqlite3 not found])])


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])
# AC_CHECK_HEADERS([fuse.h],, [AC_MSG_ERROR([fuse.h not found])])
AC_CHECK_HEADERS([sqlite3.h],, [AC_MSG_ERROR([sqlite3.h not found])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([fchdir fdatasync ftruncate gettimeofday lchown memset mkdir mkfifo realpath rmdir strdup])

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 src/Makefile
                 src/libmonfs/Makefile
		 src/monfs/Makefile])
AC_OUTPUT
