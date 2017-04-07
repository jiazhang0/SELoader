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

#include "Internal.h"

EFI_GUID gEfiMok2VerifyProtocolGuid = EFI_MOK2_VERIFY_PROTOCOL_GUID;

STATIC EFI_HANDLE Mok2VerifyHandle;

STATIC EFI_STATUS EFIAPI
Mok2VerifySignature(IN EFI_MOK2_VERIFY_PROTOCOL *This, IN VOID *Signature,
		    IN UINTN SignatureSize, IN VOID *Data, IN UINTN DataSize)
{
	if (!This || !Signature || !SignatureSize)
		return EFI_INVALID_PARAMETER;

	if (DataSize && !Data)
		return EFI_INVALID_PARAMETER;

	EfiConsoleTraceDebug(L"Attempting to verify signature by MOK2 Verify "
			     L"Protocol ...");

	if (EfiSecurityPolicySecureBootEnabled() == FALSE) {
		EfiConsoleTraceDebug(L"Ignore to verify signature\n");
		return EFI_SUCCESS;
	}

	EFI_STATUS Status;

	Status = MokVerifyPeImage(Data, DataSize);
	if (EFI_ERROR(Status)) {
		Status = EfiImageLoad(NULL, Data, DataSize);
		if (!EFI_ERROR(Status))
			EfiConsoleTraceDebug(L"Succeeded to verify signature "
					     L"by MOK2 Verify Protocol\n");
	} else
		EfiConsoleTraceDebug(L"Succeeded to verify signature by MOK "
				     L"Verify Protocol\n");

	return Status;
}

STATIC EFI_STATUS EFIAPI
Mok2VerifyFileBuffer(IN EFI_MOK2_VERIFY_PROTOCOL *This, IN OUT VOID **Data,
		     IN OUT UINTN *DataSize, IN CONST CHAR16 *Path)
{
	if (!This || !Data || !DataSize || !Path)
		return EFI_INVALID_PARAMETER;

	EfiConsoleTraceDebug(L"Attempting to verify file buffer %s by MOK2 "
			     L"Verify Protocol ...\n", Path);

	return EFI_UNSUPPORTED;
}

STATIC EFI_STATUS EFIAPI
Mok2VerifyFile(IN EFI_MOK2_VERIFY_PROTOCOL *This, IN CONST CHAR16 *Path)
{
	if (!This || !Path)
		return EFI_INVALID_PARAMETER;

	EfiConsoleTraceDebug(L"Attempting to verify file %s by MOK2 Verify "
			     L"Protocol ...", Path);

	VOID *Data = NULL;
	UINTN DataSize = 0;
	EFI_STATUS Status;

	Status = EfiFileLoad(Path, &Data, &DataSize);
	if (!EFI_ERROR(Status))
		EfiConsoleTraceDebug(L"Succeeded to verify file %s by MOK2 "
				     L"Verify Protocol\n", Path);
	else
		EfiConsoleTraceDebug(L"Failed to verify file %s by MOK2 "
				     L"Verify Protocol\n", Path);

	return Status;
}

STATIC EFI_MOK2_VERIFY_PROTOCOL Mok2VerifyProtocol = {
	1,
	Mok2VerifySignature,
	Mok2VerifyFileBuffer,
	Mok2VerifyFile
};

EFI_STATUS
Mok2VerifyInitialize(VOID)
{
	UINT8 MokSBState = 1;
	EFI_STATUS Status;

	Status = MokSecureBootState(&MokSBState);
        if (!EFI_ERROR(Status)) {
		if (MokSBState == 1) {
        		EfiConsolePrintDebug(L"Ignore to install MOK2 Verify "
        				     L"Protocol\n");
        		return EFI_SUCCESS;
		}	
        }

	Status = EfiProtocolInstall(&Mok2VerifyHandle,
				    &gEfiMok2VerifyProtocolGuid,
				    (VOID *)&Mok2VerifyProtocol);
        if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to install MOK2 Verify "
				     L"Protocol (err: 0x%x)\n", Status);
		return Status;
        }

	EfiConsolePrintDebug(L"MOK2 Verify Protocol installed\n");

        return EFI_SUCCESS;
}
