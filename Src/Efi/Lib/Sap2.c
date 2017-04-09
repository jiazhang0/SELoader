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

#include "Internal.h"

EFI_GUID gEfiSecurity2ArchProtocolGuid = EFI_SECURITY2_ARCH_PROTOCOL_GUID;

STATIC EFI_SECURITY2_ARCH_PROTOCOL *Security2ArchProtocol;
STATIC EFI_SECURITY2_FILE_AUTHENTICATION OriginalFileAuthentication;

EFI_STATUS EFIAPI
ReplacedFileAuthentication(IN CONST EFI_SECURITY2_ARCH_PROTOCOL *This,
			   IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
			   IN VOID *FileBuffer,
			   IN UINTN FileSize,
			   IN BOOLEAN BootPolicy)
{
	EfiConsoleTraceDebug(L"The FileAuthentication() hook called\n");

	EFI_STATUS Status;

	Status = MokVerifyPeImage(FileBuffer, FileSize);
	if (!EFI_ERROR(Status)) {
		EfiConsoleTraceDebug(L"The FileAuthentication() hook called\n");
		return EFI_SUCCESS;
	}

	/* Chain original security policy */
	Status = OriginalFileAuthentication(This, DevicePath, FileBuffer,
					    FileSize, BootPolicy);
	if (EFI_ERROR(Status)) {
		EfiConsoleTraceError(L"Failed to verify PE image by the "
				     L"original FileAuthentication()\n");
		return Status;
	}

	EfiConsoleTraceDebug(L"Succeeded to call the FileAuthentication() "
			     L"hook\n");

	return EFI_SUCCESS;
}

EFI_STATUS
HookSap2(VOID)
{
	EFI_STATUS Status;

	Status = EfiProtocolLocate(&gEfiSecurity2ArchProtocolGuid,
				   (VOID **)&Security2ArchProtocol);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to open Security2 Architectural "
				     L"Protocol\n");
		return Status;
	}

	OriginalFileAuthentication = Security2ArchProtocol->FileAuthentication;
	Security2ArchProtocol->FileAuthentication = ReplacedFileAuthentication;

	/* Check to see whether the interface is in write protected memory */
	if (Security2ArchProtocol->FileAuthentication !=
	    ReplacedFileAuthentication) {
		OriginalFileAuthentication = NULL;
		EfiConsolePrintError(L"Unable to hook Security2 Architectural "
				     L"Protocol\n");
		return EFI_ACCESS_DENIED;
	}

	EfiConsolePrintDebug(L"Succeeded to hook Security2 Architectural "
			     L"Protocol\n");

	return EFI_SUCCESS;
}
