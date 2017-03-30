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
STATIC EFI_SIGNATURE_LIST **AllowedDb;
STATIC EFI_SIGNATURE_LIST **RevokedDb;
/* TODO: Support Dbt */
STATIC EFI_SIGNATURE_LIST *TimeStampDb[1] = { NULL };

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
	EFI_STATUS Status;

	Status = EfiMemoryAllocate(Size, (VOID **)&List);
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

STATIC EFI_STATUS
InitializePkcs7(VOID)
{
	EfiConsoleTraceDebug(L"Initializing PKCS#7 infrastructure ...");

	EFI_STATUS Status;

	Status = EfiProtocolLocate(&gEfiPkcs7VerifyProtocolGuid,
				   (VOID **)&Pkcs7VerifyProtocol);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintInfo(L"PKCS#7 Verify Protocol installed "
				    L"by BIOS\n");
		return EFI_SUCCESS;
	}

	EfiConsolePrintDebug(L"PKCS#7 Verify Protocol not supported by BIOS.\n"
			     L"Attempting to load Pkcs7VerifyDxe driver "
			     L"...\n");

	Status = EfiImageExecuteDriver(L"Pkcs7VerifyDxe.efi");
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to load Pkcs7VerifyDxe driver "
				     L"(err: 0x%x)\n", Status);
		return Status;
	}

	Status = EfiProtocolLocate(&gEfiPkcs7VerifyProtocolGuid,
				   (VOID **)&Pkcs7VerifyProtocol);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Still unable to find PKCS#7 Verify "
				     L"Protocol (err: 0x%x)\n", Status);
		return Status;
	}

	EFI_SIGNATURE_LIST *Db = NULL;
	UINTN DbSize = 0;

	Status = EfiSecurityPolicyLoad(L"db", &Db, &DbSize);
	if (EFI_ERROR(Status))
		return Status;

	EFI_SIGNATURE_LIST *MokList = NULL;
	UINTN MokListSize = 0;

	Status = EfiSecurityPolicyLoad(L"MokList", &MokList, &MokListSize);
	if (EFI_ERROR(Status) && Status != EFI_NOT_FOUND)
		goto ErrorOnLoadMokList;

	EFI_SIGNATURE_LIST *Dbx = NULL;
	UINTN DbxSize = 0;

	Status = EfiSecurityPolicyLoad(L"dbx", &Dbx, &DbxSize);
	if (EFI_ERROR(Status) && Status != EFI_NOT_FOUND)
		goto ErrorOnLoadDbx;

	EFI_SIGNATURE_LIST *MokListX = NULL;
	UINTN MokListXSize = 0;

	Status = EfiSecurityPolicyLoad(L"MokListX", &MokListX, &MokListXSize);
	if (EFI_ERROR(Status) && Status != EFI_NOT_FOUND)
		goto ErrorOnLoadMokListX;

	Status = MergeSignatureList(&AllowedDb, Db, DbSize, MokList,
				    MokListSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to merge the allowed "
				     L"database (err: 0x%x)\n");
		goto ErrorMergeAllowedDb;
	}

	Status = MergeSignatureList(&RevokedDb, Dbx, DbxSize, MokListX,
				    MokListXSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to merge the revoked "
				     L"database (err: 0x%x)\n");
		goto ErrorMergeRevokedDb;
	}	

	Pkcs7Initialized = TRUE;

	EfiConsoleTraceInfo(L"PKCS#7 Verify Protocol loaded");

	return EFI_SUCCESS;

ErrorMergeRevokedDb:
	EfiMemoryFree(AllowedDb);

ErrorMergeAllowedDb:
	EfiMemoryFree(MokListX);

ErrorOnLoadMokListX:
	EfiMemoryFree(Dbx);

ErrorOnLoadDbx:
	EfiMemoryFree(MokList);

ErrorOnLoadMokList:
	EfiMemoryFree(Db);

	return Status;
}

EFI_STATUS
Pkcs7VerifyDetachedSignature(VOID *Hash, UINTN HashSize,
			     VOID *Signature, UINTN SignatureSize)
{
	if (!Hash || !HashSize || !Signature || !SignatureSize)
		return EFI_INVALID_PARAMETER;

	EFI_STATUS Status;

	if (Pkcs7Initialized == FALSE) {
		Status = InitializePkcs7();
		if (EFI_ERROR(Status))
			return Status;
	}

	/*
	 * NOTE: Current EDKII-OpenSSL interface cannot support VerifySignature
	 * directly. EFI_UNSUPPORTED is always returned.
	 */
#if 0
	Status = Pkcs7VerifyProtocol->VerifySignature(Pkcs7VerifyProtocol,
						      Signature,
						      SignatureSize,
						      Data, DataSize,
						      AllowedDb,
						      RevokedDb,
						      TimeStampDb);
#endif

	EFI_PKCS7_VERIFY_BUFFER Verify = Pkcs7VerifyProtocol->VerifyBuffer;

	Status = Verify(Pkcs7VerifyProtocol, Signature, SignatureSize,
			Hash, HashSize, AllowedDb, RevokedDb, TimeStampDb,
			NULL, NULL);
	if (!EFI_ERROR(Status))
		EfiConsolePrintDebug(L"Succeeded to verify detached PKCS#7 "
				     L"signature\n");
	else
		EfiConsolePrintError(L"Failed to verify detached PKCS#7 "
				     L"signature (err: 0x%x)\n", Status);

	return Status;
}

EFI_STATUS
Pkcs7VerifyAttachedSignature(VOID **SignedContent, UINTN *SignedContentSize,
			     VOID *Signature, UINTN SignatureSize)
{
	if (!Signature || !SignatureSize)
		return EFI_INVALID_PARAMETER;

	if (SignedContent && !SignedContentSize)
		return EFI_INVALID_PARAMETER;

	if (SignedContent && *SignedContent && SignedContentSize &&
	    !*SignedContentSize)
		return EFI_INVALID_PARAMETER;

	if (!SignedContent && SignedContentSize && *SignedContentSize)
		return EFI_INVALID_PARAMETER;

	EFI_STATUS Status;

	if (Pkcs7Initialized == FALSE) {
		Status = InitializePkcs7();
		if (EFI_ERROR(Status))
			return Status;
	}

	EFI_PKCS7_VERIFY_BUFFER Verify = Pkcs7VerifyProtocol->VerifyBuffer;
	UINT8 FixedContent[sizeof(EFI_HASH2_OUTPUT)];
	UINTN ExtractedContentSize = sizeof(FixedContent);
	UINT8 *ExtractedContent = FixedContent;

	Status = Verify(Pkcs7VerifyProtocol, Signature, SignatureSize,
			NULL, 0, AllowedDb, RevokedDb, TimeStampDb,
			ExtractedContent, &ExtractedContentSize);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		Status = EfiMemoryAllocate(ExtractedContentSize,
					   (VOID **)&ExtractedContent);
		if (!EFI_ERROR(Status))
			Status = Verify(Pkcs7VerifyProtocol, Signature,
					SignatureSize, NULL, 0, AllowedDb,
					RevokedDb, TimeStampDb,
					ExtractedContent,
					&ExtractedContentSize);
		else {
			EfiConsolePrintError(L"Unable to retrieve the signed "
					     L"content in PKCS#7 attached "
					     L"signature\n");
			return Status;
		}
	}

	if (EFI_ERROR(Status)) {
		if (ExtractedContent != FixedContent)
			EfiMemoryFree(ExtractedContent);

		EfiConsolePrintError(L"Failed to verify PKCS#7 signature "
				     L"(err: 0x%x)\n", Status);

		return Status;
	}

	EfiLibraryHexDump(L"Signed content extracted",
			  ExtractedContent, ExtractedContentSize);

	if (SignedContent && *SignedContent && *SignedContentSize) {
		MemCpy(*SignedContent, ExtractedContent,
		       MIN(*SignedContentSize, ExtractedContentSize));

		if (ExtractedContent != FixedContent)
			EfiMemoryFree(ExtractedContent);

		*SignedContentSize = MIN(*SignedContentSize,
					 ExtractedContentSize);
	} else {
		if (SignedContent)
			*SignedContent = ExtractedContent;

		if (SignedContentSize) {
			if (!*SignedContentSize)
				*SignedContentSize = ExtractedContentSize;
			else
				*SignedContentSize = MIN(*SignedContentSize,
							 ExtractedContentSize);
		}
	}

	EfiConsolePrintDebug(L"Succeeded to verify PKCS#7 attached "
			     L"signature (signed content %d-byte)\n",
			     ExtractedContentSize);

	return Status;
}
