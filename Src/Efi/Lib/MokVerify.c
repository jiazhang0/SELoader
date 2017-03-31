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

EFI_GUID gEfiMokVerifyProtocolGuid = EFI_MOK_VERIFY_PROTOCOL_GUID;

EFI_STATUS
MokVerifyPeImage(VOID *Data, UINTN DataSize)
{
	EFI_MOK_VERIFY_PROTOCOL *MokVerifyProtocol;
	EFI_STATUS Status;

	Status = EfiProtocolLocate(&gEfiMokVerifyProtocolGuid,
				   (VOID **)&MokVerifyProtocol);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintDebug(L"Failed to open MOK Verify Protocol "
				     L"(err: 0x%x)\n", Status);
		return Status;
	}

	EfiConsolePrintLevel Level;

	Status = EfiConsoleGetVerbosity(&Level);
	if (!EFI_ERROR(Status) && Level == CPL_DEBUG) {
		PE_COFF_LOADER_IMAGE_CONTEXT Context;

		Status = MokVerifyProtocol->Context(Data, DataSize, &Context);
		if (!EFI_ERROR(Status)) {
			EfiConsolePrintDebug(L"Succeeded to construct the context for "
					     L"PE image\n");

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
	}

	Status = MokVerifyProtocol->Verify(Data, DataSize);
	if (!EFI_ERROR(Status))
		EfiConsoleTraceDebug(L"Succeeded to verify PE image\n");
	else
		EfiConsoleTraceError(L"Failed to verify PE image "
				     L"(err: 0x%x)\n", Status);

	return Status;
}

EFI_STATUS
MokVerifyProtocolInstalled(BOOLEAN *Installed)
{
	if (!Installed)
		return EFI_INVALID_PARAMETER;

	EFI_STATUS Status;

	Status = EfiProtocolLocate(&gEfiMokVerifyProtocolGuid, NULL);
	if (EFI_ERROR(Status)) {
		*Installed = FALSE;
		return Status;
	}

	*Installed = TRUE;

        return EFI_SUCCESS;
}

EFI_STATUS
MokSecureBootState(UINT8 *MokSBState)
{
	if (!MokSBState)
		return EFI_INVALID_PARAMETER;

	UINT32 Attributes;
	UINTN VarSize = sizeof(*MokSBState);
	EFI_STATUS Status;

	Status = EfiVariableReadMok(L"MokSBState", &Attributes,
				    (VOID **)&MokSBState, &VarSize);
	if (EFI_ERROR(Status)) {
		if (Status == EFI_NOT_FOUND) {
			*MokSBState = 0;

			EfiConsolePrintDebug(L"Assuming MOK Secure Boot "
					     L"enabled\n");

			return EFI_SUCCESS;
		}

		EfiConsolePrintError(L"Failed to read MokSBState "
				     L"(err: 0x%x)\n", Status);

		return Status;
	}

	Status = EFI_UNSUPPORTED;

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid size of MokSBState (%d-byte)\n",
				     VarSize);
		goto Err;
	}

	if (Attributes != EFI_VARIABLE_BOOTSERVICE_ACCESS) {
		EfiConsolePrintError(L"Invalid attribute of MokSBState "
				     L"(0x%x)\n", Attributes);
		goto Err;
	}

	if (*MokSBState != 0 && *MokSBState != 1) {
		EfiConsolePrintError(L"Invalid value of MokSBState (0x%x)\n",
				     *MokSBState);
		goto Err;
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteMok(L"MokSBState");

	return Status;
}
