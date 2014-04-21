#FIXME: Pass -Werror to submakes

SKIPDIRS ?= mpu flight-imu eventlogger
SUBDIRS = $(shell find -mindepth 1 -maxdepth 1 -type d $(foreach dir,$(SKIPDIRS),-not -name "$(dir)"))
CLEAN_SUBDIRS = $(SUBDIRS:%=clean-%)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ -w

clean: $(CLEAN_SUBDIRS)

$(CLEAN_SUBDIRS): 
	$(MAKE) -C $(@:clean-%=%) -w clean

rebuild: clean all

.PHONY: all clean $(SUBDIRS) $(CLEAN_SUBDIRS)
