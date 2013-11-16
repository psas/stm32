SUBDIRS = $(shell ls -d */)
CLEAN_SUBDIRS = $(SUBDIRS:%=install-%)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ -w

clean: $(CLEAN_SUBDIRS)

$(CLEAN_SUBDIRS): 
	$(MAKE) -C $(@:install-%=%) -w clean

.PHONY: all clean $(SUBDIRS) $(CLEAN_SUBDIRS)
