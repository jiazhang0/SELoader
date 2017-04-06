CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
OBJCOPY := $(CROSS_COMPILE)objcopy
NM := $(CROSS_COMPILE)nm
INSTALL ?= install
OPENSSL ?= openssl
GIT ?= git
SBSIGN ?= sbsign
RM ?= rm

EXTRA_CFLAGS ?=
EXTRA_LDFLAGS ?=

DEBUG_BUILD ?=
TRACE_BUILD ?=

LIB_DIR := $(TOPDIR)/Src/Efi/Lib
# Installation location for SELoader.efi
EFI_DESTDIR ?= /boot/efi/EFI/BOOT

prefix ?= /usr
libdir ?= $(prefix)/lib
bindir ?= $(prefix)/bin
sbindir ?= $(prefix)/sbin
includedir ?= $(prefix)/include

gnuefi_libdir ?= $(libdir)
GNU_EFI_VERSION ?= 304

LIB_GCC ?= $(shell $(CC) -print-libgcc-file-name)

LDFLAGS := --warn-common --no-undefined --fatal-warnings \
	   $(patsubst $(join -Wl,,)%,%,$(EXTRA_LDFLAGS))
CFLAGS := -std=gnu11 -Wall -Wsign-compare -Werror \
	  $(EXTRA_CFLAGS) $(addprefix $(join -Wl,,),$(LDFLAGS))

SIGNING_KEY ?= $(TOPDIR)/Key/mok_sb_keys/vendor_cert.key
SIGNING_CERT ?= $(TOPDIR)/Key/mok_sb_keys/vendor_cert.pem

ifneq ($(DEBUG_BUILD),)
	CFLAGS += -ggdb -DDEBUG_BUILD
endif

ifneq ($(TRACE_BUILD),)
	CFLAGS += -DTRACE_BUILD
endif
