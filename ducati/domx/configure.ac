AC_PREREQ(2.52)

dnl initialize autoconf
AC_INIT([domx], [1.0.0], [http://www.ti.com])

dnl define PACKAGE_VERSION_* variables
dnl AS_VERSION

dnl can autoconf find the source ?
AC_CONFIG_SRCDIR([system/domx/omx_proxy_common/src/omx_proxy_common.c])

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
dnl Check for OMX Core:
PKG_CHECK_MODULES([OMXCORE], [libOMX_Core])

dnl Check for OMX mm_osal
PKG_CHECK_MODULES([MMOSAL], [libmmosal])

dnl Check for syslink
PKG_CHECK_MODULES([SYSLINK], [syslink])

dnl Check for tiler memmgr
PKG_CHECK_MODULES([MEMMGR], [libmemmgr])

dnl Check for d2cmap
PKG_CHECK_MODULES([D2CMAP], [libd2cmap])

dnl *** checks for header files ***
dnl check if we have ANSI C header files
AC_HEADER_STDC

dnl *** checks for types/defines ***

dnl *** checks for structures ***

dnl *** checks for compiler characteristics ***

dnl *** checks for library functions ***

dnl *** checks for dependancy libraries ***

dnl *** set variables based on configure arguments ***
dnl *** set variables based on configure arguments ***
dnl check for enabled testapp build
AC_ARG_ENABLE(tests,
        AC_HELP_STRING([--enable-tests], [Build test app(s) (default: disabled)]),
        [case "${enableval}" in
              yes) enable_tests=true ;;
              no)  enable_tests=false ;;
              *) AC_MSG_ERROR(bad value ${enableval} for --enable-tests)
                ;;
        esac], [enable_tests=false])
AM_CONDITIONAL([BUILD_TESTS], test "x$enable_tests" = "xtrue")


AC_CONFIG_FILES([
Makefile
mmplatform/src/Makefile
system/domx/Makefile
system/domx/omx_proxy_common/src/Makefile
system/domx/omx_rpc/src/Makefile
video/omx_proxy_component/src/Makefile
test/camera/Makefile
test/h264dec/Makefile
test/h264play/Makefile
test/h264play2/Makefile
test/h264enc/Makefile
test/jpegdec/Makefile
test/sample_proxy/Makefile
])
AC_OUTPUT
