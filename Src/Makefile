include $(TOPDIR)/Rules.mk

SUBDIRS := Efi

all clean install:
	@for x in $(SUBDIRS); do $(MAKE) -C $$x $@ || exit $?; done
