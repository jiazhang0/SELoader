/*
 * Copyright (c) 2017, Wind River Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Author:
 *        Lans Zhang <jia.zhang@windriver.com>
 */

#include <Efi.h>
#include <EfiLibrary.h>

#include "Internal.h"

EFI_GUID gEfiSecurity2ArchProtocolGuid = EFI_SECURITY2_ARCH_PROTOCOL_GUID;

STATIC EFI_SECURITY2_FILE_AUTHENTICATION OriginalFileAuthentication;

EFI_STATUS EFIAPI
ReplacedFileAuthentication(IN CONST EFI_SECURITY2_ARCH_PROTOCOL *This,
			   IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
			   IN VOID *FileBuffer,
			   IN UINTN FileSize,
			   IN BOOLEAN BootPolicy)
{
	CHAR16 *FilePath = DevicePathToStr((EFI_DEVICE_PATH *)DevicePath);

	if (!FilePath)
		return EFI_OUT_OF_RESOURCES;

	EfiConsoleTraceDebug(L"The FileAuthentication() hook called for "
			     L"authenticating %s\n", FilePath);

	BOOLEAN Installed;
	EFI_STATUS Status;

	Status = MokVerifyProtocolInstalled(&Installed);
	if (EFI_ERROR(Status))
		return Status;

	if (Installed == TRUE) {
		Status = MokVerifyPeImage(FileBuffer, FileSize);
		if (!EFI_ERROR(Status)) {
			EfiConsoleTraceDebug(L"Succeeded to verify PE image "
					     L"by the FileAuthentication() "
					     L"hook\n");
			return EFI_SUCCESS;
		}
	}

	EfiConsolePrintDebug(L"Falling back to the original "
			     L"FileAuthentication() ...\n");

	/* Chain original security policy */
	Status = OriginalFileAuthentication(This, DevicePath, FileBuffer,
					    FileSize, BootPolicy);
	if (EFI_ERROR(Status)) {
		EfiConsoleTraceError(L"Failed to verify PE image %s by the "
				     L"original FileAuthentication()\n",
				     FilePath);
		return Status;
	}

	EfiConsoleTraceDebug(L"Succeeded to verify PE image %s by the "
			     L"original FileAuthentication()\n", FilePath);

	return EFI_SUCCESS;
}

EFI_STATUS
Sap2Hook(VOID)
{
	EFI_SECURITY2_ARCH_PROTOCOL *Sap2;
	EFI_STATUS Status;

	Status = EfiProtocolLocate(&gEfiSecurity2ArchProtocolGuid,
				   (VOID **)&Sap2);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to open EFI Security2 "
				     L"Architectural Protocol\n");
		return Status;
	}

	OriginalFileAuthentication = Sap2->FileAuthentication;
	Sap2->FileAuthentication = ReplacedFileAuthentication;

	/* Check to see whether the interface is in write protected memory */
	if (Sap2->FileAuthentication != ReplacedFileAuthentication) {
		EfiConsolePrintError(L"Unable to hook EFI Security2 "
				     L"Architectural Protocol\n");
		return EFI_ACCESS_DENIED;
	}

	EfiConsolePrintDebug(L"Succeeded to hook EFI Security2 Architectural "
			     L"Protocol\n");

	return EFI_SUCCESS;
}
