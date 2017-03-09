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
#include <SELoader.h>

#include "Internal.h"

typedef struct {
	UINTN Revision;	
	UINTN SignedDataLength;
	VOID *Pkcs7Signature;
	UINTN Pkcs7SignatureSize;
	EFI_GUID *HashAlgorithm;
	UINTN Flags;
} SEL_SIGNATURE_CONTEXT;

STATIC EFI_STATUS
ParseSignature(VOID *Signature, UINTN SignatureSize,
	       SEL_SIGNATURE_CONTEXT *Context)
{
	Context->HashAlgorithm = MemDup(&gEfiHashAlgorithmSha256Guid,
					sizeof(gEfiHashAlgorithmSha256Guid));
	if (!Context->HashAlgorithm)
		return EFI_OUT_OF_RESOURCES;

	Context->Revision = SelSignatureRevision;
	Context->SignedDataLength = 0;
	Context->Pkcs7Signature = Signature;
	Context->Pkcs7SignatureSize = SignatureSize;
	Context->Flags = SelSignatureTagSignaturePkcs7FlagsDetached;

	return EFI_SUCCESS;
}

EFI_STATUS
EfiSignatureVerify(VOID *Signature, UINTN SignatureSize,
		   VOID *Data, UINTN DataSize)
{
	if (!Signature || !SignatureSize || !Data || !DataSize)
		return EFI_INVALID_PARAMETER;

	SEL_SIGNATURE_CONTEXT SignatureContext;
	EFI_STATUS Status = ParseSignature(Signature, SignatureSize,
					   &SignatureContext);
	if (EFI_ERROR(Status))
		return Status;

	UINT8 *Hash;
	UINTN HashSize;

	BOOLEAN DetachedSignature;
	DetachedSignature = !!(SignatureContext.Flags &
			       SelSignatureTagSignaturePkcs7FlagsDetached);
	if (DetachedSignature == TRUE) {
		Status = EfiHashData(SignatureContext.HashAlgorithm,
				     Data, DataSize, &Hash, &HashSize);
		if (EFI_ERROR(Status))
			return Status;
	} else {
		EFI_HASH_CONTEXT HashContext;

		Status = EfiHashInitialize(SignatureContext.HashAlgorithm,
					   &HashContext);
		if (EFI_ERROR(Status))
			return Status;

		Status = EfiHashUpdate(&HashContext, Data, DataSize);
		if (EFI_ERROR(Status)) {
			EfiHashFinalize(&HashContext, NULL, 0);
			return Status;
		}

		Status = EfiHashUpdate(&HashContext, Signature,
				       SignatureContext.SignedDataLength);
		if (EFI_ERROR(Status)) {
			EfiHashFinalize(&HashContext, NULL, 0);
			return Status;
		}

		EfiHashFinalize(&HashContext, &Hash, &HashSize);
	}

	EfiLibraryHexDump(L"Signed content hash", Hash, HashSize);

	Status = Pkcs7Verify(Hash, HashSize, SignatureContext.Pkcs7Signature,
			     SignatureContext.Pkcs7SignatureSize,
			     DetachedSignature);
	EfiMemoryFree(Hash);

	return Status;
}
