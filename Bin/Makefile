include $(TOPDIR)/Env.mk
include $(TOPDIR)/Rules.mk

all: Pkcs7VerifyDxe.efi.signed Hash2DxeCrypto.efi.signed

clean:
	@$(RM) -f *.efi.signed

install: all

Pkcs7VerifyDxe.efi.signed: Pkcs7VerifyDxe.efi
Hash2DxeCrypto.efi.signed: Hash2DxeCrypto.efi
