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
	SEL_SIGNATURE_HEADER *Signature;
	UINTN SignatureSize;
	VOID *Payload;
	UINTN PayloadSize;
	UINTN Flags;
} SEL_SIGNATURE_CONTEXT;

STATIC EFI_STATUS
ParseSelSignature(SEL_SIGNATURE_HEADER *Signature, UINTN SignatureSize,
		  SEL_SIGNATURE_CONTEXT *Context)
{
	Context->Revision = SelSignatureRevision;
	Context->Signature = Signature;
	Context->SignatureSize = SignatureSize;
	Context->Payload = Signature;
	Context->PayloadSize = SignatureSize;
	Context->Flags = 0;

	return EFI_SUCCESS;
}

EFI_STATUS
EfiSignatureVerifyAttached(VOID *Signature, UINTN SignatureSize,
			   VOID **Data, UINTN *DataSize)
{
	if (!Signature || !SignatureSize || !Data || !DataSize)
		return EFI_INVALID_PARAMETER;

	VOID *SelSignature = NULL;
	UINTN SelSignatureSize = 0;
	EFI_STATUS Status;

	Status = Pkcs7VerifyAttachedSignature(&SelSignature, &SelSignatureSize,
					      Signature, SignatureSize);
	if (EFI_ERROR(Status))
		return Status;

	SEL_SIGNATURE_CONTEXT SignatureContext;
	Status = ParseSelSignature(SelSignature, SelSignatureSize,
				   &SignatureContext);
	if (EFI_ERROR(Status)) {
		EfiMemoryFree(SelSignature);
		return Status;
	}

	*Data = SignatureContext.Payload;
	*DataSize = SignatureContext.PayloadSize;

	return Status;
}

EFI_STATUS
EfiSignatureVerifyBuffer(VOID *Signature, UINTN SignatureSize,
			 VOID *Data, UINTN DataSize)
{
	if (!Signature || !SignatureSize || !Data || !DataSize)
		return EFI_INVALID_PARAMETER;

	UINT8 *Hash;
	UINTN HashSize;
	EFI_STATUS Status;

	Status = EfiHashData(&gEfiHashAlgorithmSha256Guid,
			     Data, DataSize, &Hash, &HashSize);
	if (EFI_ERROR(Status))
		return Status;

	EfiLibraryHexDump(L"Signed content hash", Hash, HashSize);

	Status = Pkcs7VerifyDetachedSignature(Hash, HashSize,
					      Signature, SignatureSize);
	EfiMemoryFree(Hash);

	return Status;
}
