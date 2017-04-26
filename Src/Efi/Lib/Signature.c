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
	UINT8 Revision;
	SEL_SIGNATURE_HEADER *Header;
	UINTN HeaderSize;
	SEL_SIGNATURE_TAG *TagDirectory;
	UINTN NumberOfTag;
	UINTN TagDirectorySize;
	UINT8 *Payload;
	UINTN PayloadSize;
	UINT8 *Content;
	UINTN ContentSize;
	EFI_GUID *HashAlgorithm;
	UINT8 *Signature;
	UINTN SignatureSize;
} SEL_SIGNATURE_CONTEXT;

STATIC VOID
InitContext(SEL_SIGNATURE_CONTEXT *Context)
{
	MemSet(Context, 0, sizeof(*Context));
}

STATIC EFI_STATUS
ParseHashAlgorithm(SEL_SIGNATURE_HASH_ALGORITHM HashAlg,
		   SEL_SIGNATURE_CONTEXT *Context)
{
	switch (HashAlg) {
	case SelHashAlgorithmSha1:
		Context->HashAlgorithm = &gEfiHashAlgorithmSha1Guid;
		break;
	case SelHashAlgorithmSha224:
		Context->HashAlgorithm = &gEfiHashAlgorithmSha224Guid;
		break;
	case SelHashAlgorithmSha256:
		Context->HashAlgorithm = &gEfiHashAlgorithmSha256Guid;
		break;
	case SelHashAlgorithmSha384:
		Context->HashAlgorithm = &gEfiHashAlgorithmSha384Guid;
		break;
	case SelHashAlgorithmSha512:
		Context->HashAlgorithm = &gEfiHashAlgorithmSha512Guid;
		break;
	default:
		EfiConsolePrintError(L"Unsupported hash algorithm (0x%x)\n",
				     HashAlg);
		return EFI_UNSUPPORTED;
	}

	return EFI_SUCCESS;
}

STATIC EFI_STATUS
ParseHeader(UINT8 *Signature, UINTN SignatureSize,
		     SEL_SIGNATURE_CONTEXT *Context)
{
	SEL_SIGNATURE_HEADER *Header = (SEL_SIGNATURE_HEADER *)Signature;

	if (SignatureSize <= sizeof(SEL_SIGNATURE_HEADER)) {
		EfiConsolePrintError(L"Invalid signature header\n");
		return EFI_UNSUPPORTED;
	}

	if (MemCmp(&Header->Magic, SelSigantureMagic, sizeof(Header->Magic))) {
		EfiConsolePrintError(L"Unrecognized signature format\n");
		return EFI_UNSUPPORTED;
	}

	EfiConsolePrintDebug(L"Signature format revision %d supported\n",
			     SelSignatureRevision);

	if (Header->Revision == SelSignatureRevision)
		Context->Revision = SelSignatureRevision;
	else if (!Header->Revision) {
		EfiConsolePrintDebug(L"Current signature format revision "
				     L"assumed\n");
		Context->Revision = SelSignatureRevision;
	} else if (Header->Revision < SelSignatureRevision) {
		EfiConsolePrintWarning(L"Degrade the signature format "
				       L"revision to %d\n",
				       Header->Revision);
		Context->Revision = Header->Revision;
	} else {
		EfiConsolePrintError(L"Unrecognized signature format "
				     L"revision %d\n", Header->Revision);
		return EFI_UNSUPPORTED;
	}

	if (Header->HeaderSize >= SignatureSize) {
		EfiConsolePrintError(L"Invalid signature header size\n");
		return EFI_UNSUPPORTED;
	}

	UINTN TagDirectorySize = Header->NumberOfTag *
				 sizeof(SEL_SIGNATURE_TAG);

	if (!TagDirectorySize || TagDirectorySize >
				 Header->TagDirectorySize
			      || Header->HeaderSize +
				 Header->TagDirectorySize >=
				 SignatureSize) {
		EfiConsolePrintError(L"Invalid signature tag directory "
				     L"size\n");
		return EFI_UNSUPPORTED;
	}

	Context->Header = Header;
	Context->HeaderSize = Header->HeaderSize;
	Context->TagDirectory = (SEL_SIGNATURE_TAG *)(Signature +
						      Header->HeaderSize);
	Context->NumberOfTag = Header->NumberOfTag;
	Context->TagDirectorySize = TagDirectorySize;
	Context->Payload = (UINT8 *)Context->TagDirectory +
			   Header->TagDirectorySize;
	Context->PayloadSize = SignatureSize - Header->HeaderSize -
			       TagDirectorySize;
	Context->Signature = Signature;
	Context->SignatureSize = SignatureSize;

	return EFI_SUCCESS;
}

STATIC EFI_STATUS
ParseTagDirectory(SEL_SIGNATURE_CONTEXT *Context)
{
	SEL_SIGNATURE_TAG *TagDirectory;
	EFI_STATUS Status = EFI_SUCCESS;

	TagDirectory = Context->TagDirectory;
	for (UINTN Index = 0; Index < Context->NumberOfTag; ++Index) {
		if (TagDirectory[Index].DataOffset +
		    TagDirectory[Index].DataSize > Context->PayloadSize) {
			EfiConsolePrintError(L"Invalid tag size or offset\n");
			return EFI_UNSUPPORTED;
		}

		switch (TagDirectory[Index].Tag) {
		case SelSignatureTagContent:
			Context->Content = Context->Payload +
					   TagDirectory[Index].DataOffset;
			Context->ContentSize = TagDirectory[Index].DataSize;
			break;
		case SelSignatureTagHashAlgorithm:
		{
			if (TagDirectory[Index].DataSize !=
			    sizeof(SEL_SIGNATURE_HASH_ALGORITHM)) {
				EfiConsolePrintError(L"Invalid size of hash "
						     L"algorithm\n");
				return EFI_UNSUPPORTED;
			}

			SEL_SIGNATURE_HASH_ALGORITHM HashAlg =
			  *(SEL_SIGNATURE_HASH_ALGORITHM *)(Context->Payload +
				TagDirectory[Index].DataOffset);
			Status = ParseHashAlgorithm(HashAlg, Context);
			if (EFI_ERROR(Status))
				return Status;
			break;
		}
		case SelSignatureTagSignatureAlgorithm:
		case SelSignatureTagSignature:
		case SelSignatureTagCreationTime:
		case SelSignatureTagFileName:
		case SelSignatureTagFileSize:
			break;
		}
	}

	return EFI_SUCCESS;
}

STATIC EFI_STATUS
VerifySelSignature(SEL_SIGNATURE_CONTEXT *Context, VOID **Data,
		   UINTN *DataSize)
{
	EFI_STATUS Status = EFI_SUCCESS;

	if (Context->HashAlgorithm) {
		if (!Data || !*Data) {
			EfiConsolePrintError(L"No data to be verified for "
					     L"hash calculation\n");
			return EFI_UNSUPPORTED;
		}

		if (!DataSize || !*DataSize) {
			EfiConsolePrintError(L"Invalid data size to be "
					     L"verified for hash "
					     L"calculation\n");
			return EFI_UNSUPPORTED;
		}

		if (!Context->Content) {
			EfiConsolePrintError(L"Invalid content for hash "
					     L"calculation\n");
			return EFI_UNSUPPORTED;
		}

		UINTN HashSize;

		Status = EfiHashSize(Context->HashAlgorithm, &HashSize);
		if (EFI_ERROR(Status))
			return Status;

		if (Context->ContentSize != HashSize) {
			EfiConsolePrintError(L"Invalid content size\n");
			return EFI_UNSUPPORTED;
		}

		UINT8 *Hash;

		Status = EfiHashData(Context->HashAlgorithm, *Data, *DataSize,
				     &Hash, &HashSize);
		if (EFI_ERROR(Status))
			return Status;

		if (MemCmp(Hash, Context->Content, HashSize)) {
			EfiConsolePrintError(L"Invalid content for hash "
					     L"comparison\n");
			EfiMemoryFree(Hash);
			return EFI_SECURITY_VIOLATION;
		}

		EfiMemoryFree(Hash);

		EfiConsolePrintDebug(L"Consistent hash comparison with "
				     L"SELoader signature\n");
	} else {
		if (!Context->Content || !Context->ContentSize) {
			EfiConsolePrintError(L"Invalid content for returning "
					     L"the data\n");
			return EFI_UNSUPPORTED;
		}

		if (Data && *Data && *DataSize) {
			*DataSize = MIN(*DataSize, Context->ContentSize);
			MemCpy(*Data, Context->Content, *DataSize);
		} else if (DataSize) {
			if (!*DataSize)
				*DataSize = Context->ContentSize;
			else
				*DataSize = MIN(*DataSize,
						Context->ContentSize);

			if (Data) {
				Status = EfiMemoryAllocate(*DataSize, Data);
				if (EFI_ERROR(Status))
					return Status;

				MemCpy(*Data, Context->Content, *DataSize);
				EfiConsolePrintDebug(L"Content attached in "
						     L"SELoader signature\n");
			}
		}
	}

	return Status;
}

STATIC EFI_STATUS
ParseSelSignature(UINT8 *Signature, UINTN SignatureSize,
		  SEL_SIGNATURE_CONTEXT *Context)
{
	InitContext(Context);

	EFI_STATUS Status;

	Status = ParseHeader(Signature, SignatureSize, Context);
	if (EFI_ERROR(Status))
		return Status;

	Status = ParseTagDirectory(Context);
	if (EFI_ERROR(Status))
		return Status;

	return EFI_SUCCESS;
}

EFI_STATUS
EfiSignatureVerifyAttached(VOID *Signature, UINTN SignatureSize,
			   VOID **Data, UINTN *DataSize)
{
	if (!Signature || !SignatureSize)
		return EFI_INVALID_PARAMETER;

	if (Data && !DataSize)
		return EFI_INVALID_PARAMETER;

	if (Data && *Data && DataSize && !*DataSize)
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

	Status = VerifySelSignature(&SignatureContext, Data, DataSize);
	EfiMemoryFree(SelSignature);
	if (EFI_ERROR(Status))
		return Status;

	EfiConsolePrintDebug(L"Succeeded to verify the attached signature\n");

	return EFI_SUCCESS;
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

	if (!EFI_ERROR(Status))
		EfiConsolePrintInfo(L"Succeeded to verify the detached "
				    L"signature\n");

	return Status;
}
