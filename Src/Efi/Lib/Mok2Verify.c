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
#include <MokVerify.h>
#include <Mok2Verify.h>

EFI_GUID gEfiMokVerifyProtocolGuid = EFI_MOK_VERIFY_PROTOCOL_GUID;
EFI_GUID gEfiMok2VerifyProtocolGuid = EFI_MOK2_VERIFY_PROTOCOL_GUID;

STATIC EFI_MOK_VERIFY_PROTOCOL *MokVerifyProtocol;
STATIC EFI_HANDLE Mok2VerifyHandle;
STATIC BOOLEAN MokSecureBootEnabled = FALSE;

EFI_STATUS
MokSecureBootState(UINT8 *MokSBState)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*MokSBState);
	EFI_STATUS Status;

	Status = EfiVariableReadMok(L"MokSBState", &Attributes,
				    (VOID **)&MokSBState, &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintDebug(L"Assuming MOK Secure Boot enabled\n");

		*MokSBState = 0;
		return EFI_SUCCESS;
	}

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid MokSBState variable size "
				     L"%d-byte\n", VarSize);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (Attributes != EFI_VARIABLE_BOOTSERVICE_ACCESS) {
		EfiConsolePrintError(L"Invalid MokSBState variable attribute "
				     L"0x%x\n", Attributes);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (*MokSBState != 0 && *MokSBState != 1) {
		EfiConsolePrintError(L"Invalid MokSBState variable value "
				     L"0x%x\n", *MokSBState);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	MokSecureBootEnabled = (*MokSBState == 0);

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteMok(L"MokSBState");

	return Status;
}

STATIC EFI_STATUS EFIAPI
Mok2VerifyBuffer(IN EFI_MOK2_VERIFY_PROTOCOL *This, IN VOID *Data,
		 IN UINTN DataSize, IN CONST CHAR16 *Path)
{
	if (!This || !Data || !DataSize || !Path)
		return EFI_INVALID_PARAMETER;

	EfiConsoleTraceDebug(L"Attempting to verify buffer %s by MOK2 Verify "
			     L"Protoocl ...", Path);

	if (MokSecureBootEnabled == FALSE) {
		EfiConsoleTraceDebug(L"Ignore to verify buffer for %s\n",
				     Path);
		return EFI_SUCCESS;
	}

	if (!MokVerifyProtocol) {
		EfiConsoleTraceDebug(L"Attempt to verify buffer for %s with "
				     L"LoadImage()\n", Path);
		return EfiImageLoad(Path, Data, DataSize);
	}

	PE_COFF_LOADER_IMAGE_CONTEXT Context;
	EFI_STATUS Status;

	Status = MokVerifyProtocol->Context(MokVerifyProtocol, Data,
					    DataSize, &Context);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintDebug(L"Succeeded to construct the context for "
				     L"PE image\n");

		EfiConsolePrintLevel Level;

		Status = EfiConsoleGetVerbosity(&Level);
		if (!EFI_ERROR(Status) && Level == CPL_DEBUG) {
			UINT8 Sha256Hash[32];
			UINT8 Sha1Hash[20];

			Status = MokVerifyProtocol->Hash(Data, DataSize,
							 &Context, Sha256Hash,
							 Sha1Hash);
			if (EFI_ERROR(Status)) {
				EfiConsolePrintError(L"Failed to calculate the "
						     L"hash for PE image "
						     L"(err: 0x%x)\n", Status);
				return Status;
                        }

			EfiLibraryHexDump(L"PE sha256 hash", Sha256Hash,
					  sizeof(Sha256Hash));
			EfiLibraryHexDump(L"PE sha1 hash", Sha1Hash,
					  sizeof(Sha1Hash));
		}

		Status = MokVerifyProtocol->Verify(Data, DataSize);
		if (!EFI_ERROR(Status))
			EfiConsoleTraceDebug(L"Succeeded to verify PE "
					     L"image\n");
		else
			EfiConsoleTraceError(L"Failed to verify PE image "
					     L"(err: 0x%x)\n", Status);

		return Status;
	}

	return Status;
}

STATIC EFI_STATUS EFIAPI
Mok2VerifyFile(IN EFI_MOK2_VERIFY_PROTOCOL *This, IN CONST CHAR16 *Path,
	       OUT VOID **Data, OUT UINTN *DataSize)
{
	if (!This || !Path || !Data || !DataSize)
		return EFI_INVALID_PARAMETER;

	if (*Data && *DataSize)
		return This->VerifyBuffer(This, *Data, *DataSize, Path);

	EfiConsoleTraceDebug(L"Attempting to verify file %s by MOK2 Verify "
			     L"Protoocl ...", Path);

	if (MokSecureBootEnabled == FALSE) {
		EfiConsoleTraceDebug(L"Ignore to verify file %s\n", Path);
		return EFI_SUCCESS;
	}

	return EFI_SUCCESS;
}

STATIC EFI_MOK2_VERIFY_PROTOCOL Mok2VerifyProtocol = {
	1,
	Mok2VerifyFile,
	Mok2VerifyBuffer,
};

EFI_STATUS
Mok2VerifyInitialize(VOID)
{
	EfiConsoleTraceDebug(L"Attempting to initialize MOK2 Verify "
			     L"Protoocl ...");

	EFI_STATUS Status;

        Status = EfiProtocolLocate(&gEfiMokVerifyProtocolGuid,
				   (VOID **)&MokVerifyProtocol);
        if (!EFI_ERROR(Status)) {
 		if (MokSecureBootEnabled == FALSE) {
        		EfiConsolePrintDebug(L"Ignore to install MOK2 Verify "
        				     L"Protocol\n");
        		return EFI_SUCCESS;
		}	
        } else 
		EfiConsolePrintError(L"Failed to open MOK Verify Protocol"
				     L"(err: 0x%x)\n", Status);

	Status = EfiProtocolInstall(&Mok2VerifyHandle,
				    &gEfiMok2VerifyProtocolGuid,
				    (VOID *)&Mok2VerifyProtocol);
        if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to install MOK2 Verify "
				     L"Protocol\n");
		return Status;
        }

	EfiConsoleTraceDebug(L"MOK2 Verify Protoocl installed");

        return EFI_SUCCESS;
}
