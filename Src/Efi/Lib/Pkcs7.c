/*
 * BSD 2-clause "Simplified" License
 *
 * Copyright (c) 2017, Lans Zhang <jia.zhang@windriver.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Efi.h>
#include <EfiLibrary.h>
#include <BaseLibrary.h>

EFI_GUID gEfiPkcs7VerifyProtocolGuid = EFI_PKCS7_VERIFY_PROTOCOL_GUID;

STATIC BOOLEAN Pkcs7Initialized = FALSE;
STATIC EFI_PKCS7_VERIFY_PROTOCOL *Pkcs7VerifyProtocol;

STATIC EFI_STATUS
InitializePkcs7(VOID)
{
	EFI_STATUS Status = EfiProtocolLocate(&gEfiPkcs7VerifyProtocolGuid,
					      (VOID **)&Pkcs7VerifyProtocol);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintInfo(L"Pkcs7Verify protocol installed\n");
		return EFI_SUCCESS;
	}

	EfiConsolePrintDebug(L"Attempting to load Pkcs7VerifyDxe "
			     L"driver ...\n");

	Status = EfiImageLoadDriver(L"Pkcs7VerifyDxe.efi");
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to load Pkcs7VerifyDxe driver "
				     L"(err: 0x%x)\n", Status);
		return Status;
	}

	Status = EfiProtocolLocate(&gEfiPkcs7VerifyProtocolGuid,
				   (VOID **)&Pkcs7VerifyProtocol);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Still unable to find Pkcs7Verify "
				     L"protocol (err: 0x%x)\n", Status);
		return Status;
	}

	Pkcs7Initialized = TRUE;

	EfiConsolePrintInfo(L"Pkcs7Verify protocol loaded\n");

	return EFI_SUCCESS;
}

STATIC EFI_STATUS
MergeSignatureList(EFI_SIGNATURE_LIST ***Destination,
		   EFI_SIGNATURE_LIST *Source1, UINTN SourceSize1,
		   EFI_SIGNATURE_LIST *Source2, UINTN SourceSize2)
{
	UINTN Size = sizeof(EFI_SIGNATURE_LIST *);
	UINTN OffsetSize = Size;

	if (SourceSize1) {
		OffsetSize += sizeof(EFI_SIGNATURE_LIST *);
		Size += sizeof(EFI_SIGNATURE_LIST *) + SourceSize1;
	}

	if (SourceSize2) {
		OffsetSize += sizeof(EFI_SIGNATURE_LIST *);
		Size += sizeof(EFI_SIGNATURE_LIST *) + SourceSize2;	
	}

	EFI_SIGNATURE_LIST **List;
	EFI_STATUS Status = EfiMemoryAllocate(Size, (VOID **)&List);
	if (EFI_ERROR(Status))
		return EFI_OUT_OF_RESOURCES;

	UINTN Index = 0;
	UINT8 *Pointer = (UINT8 *)List;

	if (SourceSize1) {
		List[Index] = (EFI_SIGNATURE_LIST *)(Pointer + OffsetSize);
		MemCpy(List[Index++], Source1, SourceSize1);
		OffsetSize += SourceSize1;
	}

	if (SourceSize2) {
		List[Index] = (EFI_SIGNATURE_LIST *)(Pointer + OffsetSize);
		MemCpy(List[Index++], Source2, SourceSize2);
	}

	List[Index] = NULL;
	*Destination = List;

	return EFI_SUCCESS;
}

EFI_STATUS
Pkcs7Verify(VOID *Data, UINTN DataSize,
	    VOID *Signature, UINTN SignatureSize)
{
	EFI_STATUS Status;

	if (Pkcs7Initialized == FALSE) {
		Status = InitializePkcs7();
		if (EFI_ERROR(Status))
			return Status;
	}

	EFI_SIGNATURE_LIST *Db = NULL;
	UINTN DbSize = 0;
	Status = EfiSecurityPolicyLoad(L"db", &Db, &DbSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to load the security policy "
				     L"object db\n");
		return Status;
	}

	EFI_SIGNATURE_LIST *MokList = NULL;
	UINTN MokListSize = 0;
	Status = EfiSecurityPolicyLoad(L"MokList", &MokList, &MokListSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to load the security policy "
				     L"object MokList\n");
		goto ErrorOnLoadMokList;
	}

	if (!DbSize && !MokListSize) {
		EfiConsolePrintDebug(L"Ignore the PKCS#7 verification\n");
		goto IgnoreVerification;
	}

	EFI_SIGNATURE_LIST *Dbx = NULL;
	UINTN DbxSize = 0;
	Status = EfiSecurityPolicyLoad(L"dbx", &Dbx, &DbxSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to load the security policy "
				     L"object dbx\n");
		goto ErrorOnLoadDbx;
	}

	EFI_SIGNATURE_LIST *MokListX = NULL;
	UINTN MokListXSize = 0;
	Status = EfiSecurityPolicyLoad(L"MokListX", &MokListX, &MokListXSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to load the security policy "
				     L"object MokListX\n");
		goto ErrorOnLoadMokListX;
	}

	EFI_SIGNATURE_LIST **AllowedDb;
	Status = MergeSignatureList(&AllowedDb, Db, DbSize, MokList,
				    MokListSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to merge the allowed "
				     L"database\n");
		goto ErrorMergeAllowedDb;
	}

	EFI_SIGNATURE_LIST **RevokedDb;
	Status = MergeSignatureList(&RevokedDb, Dbx, DbxSize, MokListX,
				    MokListXSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to merge the revoked "
				     L"database\n");
		goto ErrorMergeRevokedDb;
	}	

	/* TODO: Support Dbt */
        EFI_SIGNATURE_LIST *TimeStampDb[1] = { NULL };
	UINT8 Digest[32];
	UINTN DigestSize = sizeof(Digest);
	Status = Pkcs7VerifyProtocol->VerifyBuffer(Pkcs7VerifyProtocol,
						   Signature, SignatureSize,
						   NULL, 0, AllowedDb,
						   RevokedDb, TimeStampDb,
						   Digest, &DigestSize);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintDebug(L"Succeeded to verify PKCS#7 "
				     L"signature\n");

		EfiLibraryHexDump(L"Signed data extracted", Digest, DigestSize);
	} else
		EfiConsolePrintDebug(L"Failed to verify PKCS#7 signature "
				     L"(err: 0x%x)\n", Status); 

	EfiMemoryFree(RevokedDb);

ErrorMergeRevokedDb:
	EfiMemoryFree(AllowedDb);

ErrorMergeAllowedDb:
	EfiMemoryFree(MokListX);

ErrorOnLoadMokListX:
	EfiMemoryFree(Dbx);

IgnoreVerification:
ErrorOnLoadDbx:
	EfiMemoryFree(MokList);

ErrorOnLoadMokList:
	EfiMemoryFree(Db);

	return Status;
}
