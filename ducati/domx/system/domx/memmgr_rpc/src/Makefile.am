## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES      = libmemmgr_rpc.la
libmemmgr_rpc_la_SOURCES       = memmgr_rpc.c

libmemmgr_rpc_la_LIBADD        = \
	$(MMOSAL_LIBS) \
	$(OMXCORE_LIBS) \
	$(MEMMGR_LIBS) \
	$(SYSLINK_LIBS) \
	@LTLIBOBJS@

libmemmgr_rpc_la_CFLAGS        = \
	$(WARN_CFLAGS) \
	-I$(top_srcdir)/system/domx \
	-I$(top_srcdir)/system/domx/memmgr_rpc/inc \
	$(MMOSAL_CFLAGS) \
	$(OMXCORE_CFLAGS) \
	$(MEMMGR_CFLAGS) \
	$(SYSLINK_CFLAGS)

