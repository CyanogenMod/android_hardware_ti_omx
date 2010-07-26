## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES      = libsrc.la
libsrc_la_SOURCES       = OMX_Core.c
libsrc_la_LIBADD        = @LTLIBOBJS@
libsrc_la_CFLAGS        = -I. $(OMXCORE_CFLAGS) -DSTATIC_TABLE $(WARN_CFLAGS)

