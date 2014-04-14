#FIXME: Pass -Werror to submakes

SKIPDIRS = mpu flight-imu eventlogger
SUBDIRS = $(shell find -mindepth 1 -maxdepth 1 -type d $(foreach dir,$(SKIPDIRS),-not -name "$(dir)"))
CLEAN_SUBDIRS = $(SUBDIRS:%=install-%)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ -w

clean: $(CLEAN_SUBDIRS)

$(CLEAN_SUBDIRS): 
	$(MAKE) -C $(@:install-%=%) -w clean

.PHONY: all clean $(SUBDIRS) $(CLEAN_SUBDIRS)
