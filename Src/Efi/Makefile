include $(TOPDIR)/Env.mk
include $(TOPDIR)/Rules.mk

ARCH ?= $(shell $(CC) -dumpmachine | cut -f1 -d- | sed s,i[3456789]86,i386,)

# XXX: __attribute__((ms_abi)) is only available since gcc 4.7
CFLAGS += -mno-mmx -mno-sse -maccumulate-outgoing-args \
	  -fno-merge-constants -fno-strict-aliasing \
	  -ffreestanding -fshort-wchar \
	  -fno-stack-protector -fno-stack-check \
	  -fpic -fno-builtin -nostdinc \
	  $(addprefix -I, $(TOPDIR)/Src/Efi/Include \
			  $(TOPDIR)/Src/Efi/Include/Edk2 \
			  $(shell $(CC) -print-file-name=include)) \
	  -DEFIAPI=__attribute__\(\(ms_abi\)\) -DNO_MSABI_VA_FUNCS \
	  -DGNU_EFI_VERSION=$(GNU_EFI_VERSION) -DCONFIG_$(ARCH) \
	  -DGNU_EFI_USE_MS_ABI -DSEL_VERSION=L\"$(SEL_VERSION)\"

ifeq ($(ARCH),x86_64)
    EFI_ARCH = x64
    CFLAGS += -m64 -mno-red-zone -DEFI_ARCH=\"$(EFI_ARCH)\" \
              -I$(TOPDIR)/Src/Efi/Include/Edk2/X64
else ifeq ($(ARCH),i386)
    EFI_ARCH = ia32
    CFLAGS += -m32 -DEFI_ARCH=\"$(EFI_ARCH)\"
              -I$(TOPDIR)/Src/Efi/Include/Edk2/Ia32
else
    $(error Unsupported ARCH $(ARCH) specified)
endif

LD_SCRIPT = elf_$(ARCH)_efi.lds
#LD_SCRIPT = ./efi_$(EFI_ARCH).lds
LDFLAGS += -nostdlib -shared -Bsymbolic -L$(gnuefi_libdir) \
	   -T $(LD_SCRIPT) $(gnuefi_libdir)/crt0-efi-$(ARCH).o \
	   $(EXTRA_LDFLAGS)

export CC AR CFLAGS LDFLAGS

EFI_NAME := SELoader
EFI_TARGET := $(EFI_NAME).efi

LIB_TARGETS := \
	$(LIB_DIR)/libSELoader.a \
	$(LIB_DIR)/BaseLibrary/libBaseLibrary.a

OBJS_$(EFI_NAME) := \
	SELoader.o \
	BuildInfo.o

all: $(EFI_TARGET).signed Makefile

install: all
	@cp -f $^ $(EFI_DESTDIR)

clean:
	@$(RM) -f $(EFI_TARGET).signed $(EFI_TARGET) $(OBJS_$(EFI_NAME)) \
	    *.so *.debug BuildInfo.c
	@$(MAKE) -C $(LIB_DIR) clean

$(EFI_TARGET).signed: $(EFI_TARGET)

$(LIB_TARGETS):
	@$(MAKE) -C $(LIB_DIR)

$(EFI_NAME).so: $(OBJS_$(EFI_NAME)) $(LIB_TARGETS)

BuildInfo.c: BuildInfo.c.in
	sed -e "s~@@GIT_COMMIT@@~$(shell if [ -d $(TOPDIR)/.git ]; then git log -1 --pretty=format:%H | tr -d '\n'; elif [ -f $(TOPDIR)/commit ]; then cat $(TOPDIR)/commit | tr -d '\n'; else echo -n ???????; fi)~" \
		-e "s~@@BUILD_MACHINE@@~$(shell bash -c 'whoami | tr -d "\n"; echo -n @; uname=`uname -a`; echo -n $${uname//\~/_} | tr -d "\n"')~" < $^ > $@
