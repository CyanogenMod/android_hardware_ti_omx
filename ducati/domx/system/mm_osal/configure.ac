AC_PREREQ(2.52)

dnl initialize autoconf
AC_INIT([mmosal], [1.0.0], [http://www.ti.com])

dnl define PACKAGE_VERSION_* variables
dnl AS_VERSION

dnl can autoconf find the source ?
AC_CONFIG_SRCDIR([src/timm_osal.c])

dnl set the autotools aux directory
AC_CONFIG_AUX_DIR([config])

dnl AM_MAINTAINER_MODE only provides the option to configure to enable it
AM_MAINTAINER_MODE

dnl sets host_*, target_ variables
AC_CANONICAL_SYSTEM

dnl initialize automake
AM_INIT_AUTOMAKE([foreign])

dnl find a compiler
AC_PROG_CC
AM_PROG_AS
AM_PROG_LIBTOOL

dnl *** autotools stuff ****    
dnl allow for different autotools
dnl AS_AUTOTOOLS_ALTERNATE

dnl *** checks for platform ***
AC_DEFINE_UNQUOTED(CPU_`echo $target_cpu | tr a-z- A-Z_`,[1])
AC_DEFINE_UNQUOTED(OS_` echo $target_os  | tr a-z- A-Z_`,[1])

dnl *** checks for programs ***
AC_PROG_INSTALL

dnl Check for pkgconfig first
AC_CHECK_PROG([HAVE_PKGCONFIG], [pkg-config], [yes], [no])

dnl *** checks for libraries ***

dnl *** checks for header files ***
dnl check if we have ANSI C header files
AC_HEADER_STDC

dnl *** checks for types/defines ***

dnl *** checks for structures ***

dnl *** checks for compiler characteristics ***

dnl *** checks for library functions ***

dnl *** checks for dependancy libraries ***
dnl Check for TTIF (which is optional, so we'll specify ACTION-IF-FOUND and
dnl ACTION-IF-NOT-FOUND to override the default ACTION-IF-NOT-FOUND, which
dnl is fatal error):
PKG_CHECK_MODULES([TTIF], [libttif], [
        echo "TTIF found, enabling TTIF traces"
        TTIF_TRACES=-DTIMM_OSAL_LOG_OVER_TTIF=1
        TTIF_REQUIRES=libttif
        AC_DEFINE(TIMM_OSAL_LOG_OVER_TTIF)
], [
        echo "TTIF not found, disabling TTIF traces"
        TTIF_TRACES=
        TTIF_REQUIRES=
])
AC_SUBST(TTIF_TRACES)
AC_SUBST(TTIF_REQUIRES)

dnl *** set variables based on configure arguments ***


AC_CONFIG_FILES([
libmmosal.pc
Makefile
src/Makefile
])
AC_OUTPUT
