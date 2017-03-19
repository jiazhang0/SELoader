include $(TOPDIR)/Env.mk
include $(TOPDIR)/Rules.mk

all: Pkcs7VerifyDxe.efi.signed Hash2DxeCrypto.efi.signed

clean:
	@$(RM) -f *.efi.signed

install: Pkcs7VerifyDxe.efi.signed Hash2DxeCrypto.efi.signed
	@$(INSTALL) -m 0600 Pkcs7VerifyDxe.efi.signed \
	    $(EFI_DESTDIR)/Pkcs7VerifyDxe.efi
	@$(INSTALL) -m 0600 Hash2DxeCrypto.efi.signed \
	    $(EFI_DESTDIR)/Hash2DxeCrypto.efi

Pkcs7VerifyDxe.efi.signed: Pkcs7VerifyDxe.efi
Hash2DxeCrypto.efi.signed: Hash2DxeCrypto.efi
