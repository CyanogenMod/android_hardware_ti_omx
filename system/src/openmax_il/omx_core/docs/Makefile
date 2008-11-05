
HSRC=$(wildcard *.doc)
HTARGET=$(HSRC:%=$(OMXDOCDIR)/%)

include $(OMXROOT)/Master.mk

all install:: $(OMXDOCDIR) $(HTARGET) 

$(OMXDOCDIR)/%: % 
	cp -f $< $@


