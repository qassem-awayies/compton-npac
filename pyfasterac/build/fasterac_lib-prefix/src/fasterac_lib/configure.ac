#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])

# Def PACKAGE_VERSION, PACKAGE_URL, PACKAGE_BUGREPORT into C code (lib/fasterac.c)
AC_INIT(fasterac, 2.20, etasse@lpccaen.in2p3.fr, fasterac, http://faster.in2p3.fr)
AC_PREFIX_DEFAULT(/usr)
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/faster_disfast.c])
AC_CONFIG_MACRO_DIR([m4])
#AC_CONFIG_HEADERS([config.h])

# libtool init
LT_INIT

# Checks for programs.
#AC_PROG_RANLIB  # LT_INIT -> comment
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LN_S
PKG_PROG_PKG_CONFIG

# Checks for libraries.
AC_CHECK_LIB([m], [round])
AC_CHECK_LIB([z], [gzopen])

# Checks for header files.
AC_CHECK_HEADERS([math.h limits.h float.h stdlib.h string.h getopt.h zlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC

# Def SVN_VERSION into C code (lib/fasterac.c)
AC_PATH_PROG(svnver, svnversion)
if test "Xsvnver" = "X" || test `svnversion -n` = "exporté" || test `svnversion -n` = "exported"; then
  AC_DEFINE_UNQUOTED(SVN_VERSION, ["`cat saved_svn_version.txt`"], [repository svn version])
else
  AC_DEFINE_UNQUOTED(SVN_VERSION, ["`svnversion -n`"], [repository svn version])
  `svnversion -n > saved_svn_version.txt`
fi


AC_DEFINE_UNQUOTED(GIT_TAG, ["`git describe --tag`"], )

AC_CONFIG_FILES([fasterac_config.sh
                 fasterac_config.csh
                 fasterac_examples_copy
                 Makefile
                 lib/Makefile
                 src/Makefile
                 include/Makefile
                 man/Makefile
                 man/fasterac.3
                 src/fasterac_reader_code.c
                 src/dmo/autoreader.make.in
                 examples/data_reader/Makefile
                 examples/faster_to_root/Makefile
                 examples/faster_to_root/group2tree.C
                 examples/faster_to_root/group2tree_bis.C
                 examples/faster_to_root/regroup2tree.C
                 examples/misc/Makefile
                 examples/misc/TEST.sh
                 examples/oscillo/Makefile
                 examples/osciroot/rootlogon.C
                 libfasterac.pc])
#                [chmod +x fasterac_config.sh])
AC_OUTPUT
