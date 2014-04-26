#FIXME: Pass -Werror to submakes


#SKIPDIRS is intended to temporarily exclude a project from being built. Can be set from an environment variable
SKIPDIRS ?= mpu flight-imu eventlogger

#This line creates a list of all the current directories in projects and then remove those listed in SKIPDIRS:
SUBDIRS = $(shell find -mindepth 1 -maxdepth 1 -type d $(foreach dir,$(SKIPDIRS),-not -name "$(dir)"))

# Use a makefile substitution rule to create a list of files to for the *clean* target
# see http://www.gnu.org/software/make/manual/make.html#Substitution-Refs
CLEAN_SUBDIRS = $(SUBDIRS:%=clean-%)

#Default target, makes all projects
all: $(SUBDIRS)


#SUBDIRS list are the targets
#   $@ is the current target
#   -C means switch to the directory listed
#   -w means print the directory to which we are changing
$(SUBDIRS):
	$(MAKE) -C $@ -w

#Removes output from previous builds
clean: $(CLEAN_SUBDIRS)

#here $(@:clean-%=%) removes the clean- prefix from the list of files in CLEAN_SUBDIRS
$(CLEAN_SUBDIRS): 
	$(MAKE) -C $(@:clean-%=%) -w clean

rebuild: clean all

.PHONY: all clean $(SUBDIRS) $(CLEAN_SUBDIRS)
