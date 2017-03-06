include $(TOPDIR)/Version.mk

.DEFAULT_GOAL := all
.PHONE: all clean install

SUFFIXES += .efi .efi.signed

%.so: %.o
	$(LD) $(LDFLAGS) -o $@ --start-group $^ \
	    $(shell $(CC) -print-libgcc-file-name) \
	    -lgnuefi -lefi --end-group
	@echo '--------------- List unresolved symbols ---------------'
	@! $(NM) $@ | grep -iw u
	@echo '-------------------------------------------------------'

%.efi: %.so
	$(OBJCOPY) -j .text -j .sdata -j .data \
	    -j .dynamic -j .dynsym -j .rel* \
	    -j .rela* -j .reloc -j .eh_frame \
	    -j .debug_info -j .debug_abbrev -j .debug_aranges \
	    -j .debug_line -j .debug_str -j .debug_ranges \
	    -j .note.gnu.build-id \
	    $^ $@.debug
	$(OBJCOPY) -j .text -j .sdata -j .data -j .data.ident \
	    -j .dynamic -j .dynsym -j .rel* \
	    -j .rela* -j .reloc -j .eh_frame \
	    --target efi-app-$(ARCH) $^ $@

%.efi.signed: %.efi $(TOPDIR)/Key/efi_sb_keys/DB.pem \
	      $(TOPDIR)/Key/efi_sb_keys/DB.key
	@$(SBSIGN) --cert $(TOPDIR)/Key/efi_sb_keys/DB.pem \
	    --key $(TOPDIR)/Key/efi_sb_keys/DB.key \
	    $< 2>/dev/null || echo "failed to sign $<" && echo "$< signed"
