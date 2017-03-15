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

EFI_GUID gEfiHash2ServiceBindingProtocolGuid =
	EFI_HASH2_SERVICE_BINDING_PROTOCOL_GUID;	
EFI_GUID gEfiHashServiceBindingProtocolGuid =
	EFI_HASH_SERVICE_BINDING_PROTOCOL_GUID;
#if GNU_EFI_VERSION <= 303
EFI_GUID gEfiHashProtocolGuid = EFI_HASH_PROTOCOL_GUID;
#endif
EFI_GUID gEfiHash2ProtocolGuid = EFI_HASH2_PROTOCOL_GUID;
EFI_GUID gEfiHashAlgorithmSha1Guid = EFI_HASH_ALGORITHM_SHA1_GUID;
EFI_GUID gEfiHashAlgorithmSha224Guid = EFI_HASH_ALGORITHM_SHA224_GUID;
EFI_GUID gEfiHashAlgorithmSha256Guid = EFI_HASH_ALGORITHM_SHA256_GUID;
EFI_GUID gEfiHashAlgorithmSha384Guid = EFI_HASH_ALGORITHM_SHA384_GUID;
EFI_GUID gEfiHashAlgorithmSha512Guid = EFI_HASH_ALGORITHM_SHA512_GUID;

STATIC BOOLEAN HashServiceInitialized = FALSE;
STATIC EFI_SERVICE_BINDING_PROTOCOL *HashServiceBindingProtocol;
STATIC BOOLEAN Hash2ServiceBindingProtocolUsed = TRUE;
/* TODO: call DestroyChild() via notifier */
STATIC EFI_HANDLE HashHandle = NULL;
STATIC EFI_HASH2_PROTOCOL *Hash2Protocol;
STATIC EFI_HASH_PROTOCOL *HashProtocol;

STATIC EFI_STATUS
LoadHash2DxeCrypto(VOID)
{
	EFI_STATUS Status = EfiImageExecuteDriver(L"Hash2DxeCrypto.efi");
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to load Hash2DxeCrypto driver "
				     L"(err: 0x%x)\n", Status);
		if (Status == EFI_NOT_FOUND)
			EfiConsolePrintInfo(L"Try to put the "
					    L"Hash2DxeCrypto.efi to %s and "
					    L"re-launch SELoader\n",
					    gRootPath);
		else if (Status == EFI_ACCESS_DENIED)
			EfiConsolePrintInfo(L"Try to re-resign the "
					    L"Hash2DxeCrypto.efi with the "
					    L"correct private key\n");
		return Status;
	}

	Status = EfiProtocolLocate(&gEfiHash2ServiceBindingProtocolGuid,
				   (VOID **)&HashServiceBindingProtocol);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Still unable to hash2 service binding "
				     L"protocol (err: 0x%x)\n", Status);
		return Status;
	}

	EfiConsolePrintInfo(L"EFI Hash2 Protocol loaded\n");

	return EFI_SUCCESS;
}

STATIC EFI_STATUS
InitializeHashService(VOID)
{
	EfiConsoleTraceDebug(L"Initializing EFI Hash Protocol "
			     L"infrastructure ...");

	EFI_STATUS Status;

	Status = EfiProtocolLocate(&gEfiHash2ServiceBindingProtocolGuid,
				   (VOID **)&HashServiceBindingProtocol);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintDebug(L"Not found hash2 service binding "
				     L"protocol (err: 0x%x)\n", Status);
		EfiConsolePrintDebug(L"Attempting detect hash service binding "
				     L"protocol ...\n");

		Status = EfiProtocolLocate(&gEfiHashServiceBindingProtocolGuid,
					   (VOID **)&HashServiceBindingProtocol);
		if (EFI_ERROR(Status)) {
			EfiConsoleTraceDebug(L"BIOS doesn't support any hash "
					     L"service binding protocol "
					     L"(err: 0x%x).\nAttempting to "
					     L"load Hash2DxeCrypto driver "
					     L"...", Status);
			
			Status = LoadHash2DxeCrypto();
			if (EFI_ERROR(Status))
				return Status;
		} else
			Hash2ServiceBindingProtocolUsed = FALSE;
	}

	Status = HashServiceBindingProtocol->CreateChild(HashServiceBindingProtocol,
							 &HashHandle);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to create hash handle "
				     L"(err: 0x%x)\n", Status);
		return Status;
	}

	if (Hash2ServiceBindingProtocolUsed == TRUE)
		Status = EfiProtocolOpen(HashHandle, &gEfiHash2ProtocolGuid,
					 (VOID **)&Hash2Protocol);
	else
		Status = EfiProtocolOpen(HashHandle, &gEfiHashProtocolGuid,
					 (VOID **)&HashProtocol);
	if (EFI_ERROR(Status)) {
		HashServiceBindingProtocol->DestroyChild(HashServiceBindingProtocol,
							 HashHandle);
		EfiConsolePrintError(L"Unable to open EFI Hash%s Protocol "
				     L"(err: 0x%x)\n",
				     Hash2ServiceBindingProtocolUsed ? L"2" :
								       L"",
				     Status);
		return Status;
	}

	HashServiceInitialized = TRUE;

	EfiConsoleTraceDebug(L"Hash service initialized");

	return EFI_SUCCESS;
}

EFI_STATUS
EfiHashSize(CONST EFI_GUID *HashAlgorithm, UINTN *HashSize)
{
	if (!HashAlgorithm || !HashSize)
		return EFI_INVALID_PARAMETER;

	EFI_STATUS Status;

	if (HashServiceInitialized == FALSE) {
		Status = InitializeHashService();
		if (EFI_ERROR(Status))
			return Status;
	}

	if (Hash2ServiceBindingProtocolUsed == TRUE)
		return Hash2Protocol->GetHashSize(Hash2Protocol, HashAlgorithm,
						  HashSize);
	else
		return HashProtocol->GetHashSize(HashProtocol, HashAlgorithm,
						 HashSize);
}

EFI_STATUS
EfiHashInitialize(CONST EFI_GUID *HashAlgorithm,
		  EFI_HASH_CONTEXT *Context)
{
	if (!Context)
		return EFI_INVALID_PARAMETER;

	EFI_STATUS Status;

	if (HashServiceInitialized == FALSE) {
		Status = InitializeHashService();
		if (EFI_ERROR(Status))
			return Status;
	}

	UINTN HashSize;
	if (Hash2ServiceBindingProtocolUsed == TRUE)
		Status = Hash2Protocol->GetHashSize(Hash2Protocol,
						    HashAlgorithm,
					 	    &HashSize);
	else
		Status = HashProtocol->GetHashSize(HashProtocol,
						   HashAlgorithm,
					 	   &HashSize);
	if (EFI_ERROR(Status))
		return Status;

	if (Hash2ServiceBindingProtocolUsed == TRUE) {
		Status = Hash2Protocol->HashInit(Hash2Protocol,
						 HashAlgorithm);
		if (EFI_ERROR(Status))
			return Status;
	}

	Status = EfiMemoryAllocate(HashSize, (VOID **)&(Context->Hash));
	if (EFI_ERROR(Status))
		return Status;

	Context->HashAlgorithm = MemDup(HashAlgorithm,
					sizeof(*HashAlgorithm));
	if (!Context->HashAlgorithm) {
		Status = EFI_OUT_OF_RESOURCES;
		goto ErrOnAllocateHashAlgorithm;
	}

	Context->HashSize = HashSize;
	Context->Extended = FALSE;
	Context->HashedDataSize = 0;

	return EFI_SUCCESS;

ErrOnAllocateHashAlgorithm:
	EfiMemoryFree(Context->Hash);

	return Status;
}

EFI_STATUS
EfiHashUpdate(EFI_HASH_CONTEXT *Context, CONST UINT8 *Message,
	      UINTN MessageSize)
{
	if (!Context)
		return EFI_INVALID_PARAMETER;

	if (HashServiceInitialized == FALSE)
		return EFI_UNSUPPORTED;

	EFI_STATUS Status;

	if (Hash2ServiceBindingProtocolUsed == TRUE)
		Status = Hash2Protocol->HashUpdate(Hash2Protocol, Message,
						   MessageSize);
	else {
		EFI_HASH_OUTPUT HashOutput;

		*(VOID **)&HashOutput = Context->Hash;
		Status = HashProtocol->Hash(HashProtocol,
					    Context->HashAlgorithm,
					    Context->Extended, Message,
					    MessageSize, &HashOutput);
	}

	if (!EFI_ERROR(Status))
		Context->Extended = TRUE;

	Context->HashedDataSize += MessageSize;

	return Status;
}

STATIC VOID
FreeHashContext(EFI_HASH_CONTEXT *Context)
{
	EfiMemoryFree(Context->HashAlgorithm);
	EfiMemoryFree(Context->Hash);
	Context->HashAlgorithm = NULL;
	Context->Hash = NULL;
}

EFI_STATUS
EfiHashFinalize(EFI_HASH_CONTEXT *Context, UINT8 **Hash, UINTN *HashSize)
{
	if (!Context)
		return EFI_INVALID_PARAMETER;

	EFI_STATUS Status = EFI_SUCCESS;

	if (Hash) {
		Status = EfiMemoryAllocate(Context->HashSize, (VOID **)Hash);
		if (EFI_ERROR(Status))
			return Status;

		if (Hash2ServiceBindingProtocolUsed == TRUE) {
			Status = Hash2Protocol->HashFinal(Hash2Protocol,
							  (EFI_HASH2_OUTPUT *)*Hash);
			if (EFI_ERROR(Status)) {
				EfiMemoryFree(*Hash);
				goto ErrOnHashFinal;
			}
		} else
			MemCpy(*Hash, Context->Hash, Context->HashSize);

		if (HashSize)
			*HashSize = Context->HashSize;
	}

ErrOnHashFinal:
	FreeHashContext(Context);

	return Status;
}

STATIC EFI_STATUS
HashData(CONST EFI_GUID *HashAlgorithm, CONST UINT8 *Message,
	 UINTN MessageSize, UINT8 **Hash, UINTN *HashSize)
{
	EFI_HASH_CONTEXT Context;
	EFI_STATUS Status = EfiHashInitialize(HashAlgorithm, &Context);
	if (EFI_ERROR(Status))
		return Status;

	Status = EfiHashUpdate(&Context, Message, MessageSize);
	if (EFI_ERROR(Status)) {
		FreeHashContext(&Context);
		return Status;
	}

	return EfiHashFinalize(&Context, Hash, HashSize);
}

EFI_STATUS
EfiHashData(CONST EFI_GUID *HashAlgorithm, CONST UINT8 *Message,
	    UINTN MessageSize, UINT8 **Hash, UINTN *HashSize)
{
	if (!HashAlgorithm || !Hash || !HashSize)
		return EFI_INVALID_PARAMETER;

	EFI_STATUS Status;

	if (Hash2ServiceBindingProtocolUsed == TRUE) {
		Status = EfiHashSize(HashAlgorithm, HashSize);
		if (EFI_ERROR(Status))
			return Status;

		Status = EfiMemoryAllocate(*HashSize, (VOID **)Hash);
		if (EFI_ERROR(Status))
			return Status;

		Status = Hash2Protocol->Hash(Hash2Protocol, HashAlgorithm,
					     Message, MessageSize,
					     (EFI_HASH2_OUTPUT *)*Hash);
		if (EFI_ERROR(Status)) {
			EfiMemoryFree(*Hash);
			*Hash = NULL;
			return Status;
		}
	} else
		Status = HashData(HashAlgorithm, Message, MessageSize,
				  Hash, HashSize);

	return Status;
}
