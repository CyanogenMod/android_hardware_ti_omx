## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES      = libipc_setup.la
libipc_setup_la_SOURCES       = ipc_setup.c mmplatform.c
libipc_setup_la_LIBADD        = @LTLIBOBJS@ $(SYSLINK_LIBS) $(MMOSAL_LIBS)
libipc_setup_la_CFLAGS        = -I$(top_srcdir)/system/domx -I$(top_srcdir)/system/domx/omx_rpc/inc $(OMXCORE_CFLAGS) $(SYSLINK_CFLAGS) $(MMOSAL_CFLAGS)

