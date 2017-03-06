include Version.mk
include Env.mk

TOPDIR := $(shell pwd)
export TOPDIR

SUBDIRS := Bin Src

.DEFAULT_GOAL := all
.PHONE: all clean install tag

all clean install:
	@for x in $(SUBDIRS); do $(MAKE) -C $$x $@ || exit $?; done

tag:
	@$(GIT) tag -a $(SEL_VERSION) -m $(SEL_VERSION) refs/heads/master
