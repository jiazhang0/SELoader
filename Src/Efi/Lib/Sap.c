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
 *       Jia Zhang <zhang.jia@linux.alibaba.com>
 */

#include <Efi.h>
#include <EfiLibrary.h>

EFI_GUID gEfiSecurityArchProtocolGuid = EFI_SECURITY_ARCH_PROTOCOL_GUID;

STATIC EFI_SECURITY_FILE_AUTHENTICATION_STATE OriginalFileAuthenticationState;

EFI_STATUS EFIAPI
ReplacedFileAuthenticationState(IN CONST EFI_SECURITY_ARCH_PROTOCOL *This,
				IN UINT32 AuthenticationStatus,
				IN CONST EFI_DEVICE_PATH_PROTOCOL *File)
{
	EfiConsoleTraceDebug(L"The FileAuthenticationState() hook called\n");

	if (EfiSecurityPolicySecureBootEnabled() == FALSE) {
		EfiConsoleTraceDebug(L"Ignore to verify signature\n");
		return EFI_SUCCESS;
	}

	return EFI_SUCCESS;
}

EFI_STATUS
SapHook(VOID)
{
	EFI_SECURITY_ARCH_PROTOCOL *Sap;
	EFI_STATUS Status;

	/* Security Architectural Protocol should be always available */
	Status = EfiProtocolLocate(&gEfiSecurityArchProtocolGuid,
				   (VOID **)&Sap);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to open EFI Security "
				     L"Architectural Protocol\n");
		return Status;
	}

	OriginalFileAuthenticationState = Sap->FileAuthenticationState;
	Sap->FileAuthenticationState = ReplacedFileAuthenticationState;

	/* Check to see whether the interface is in write protected memory */
	if (Sap->FileAuthenticationState != ReplacedFileAuthenticationState) {
		OriginalFileAuthenticationState = NULL;
		EfiConsolePrintError(L"Unable to hook EFI Security "
				     L"Architectural Protocol\n");
		return EFI_ACCESS_DENIED;
	}

	EfiConsolePrintDebug(L"Succeeded to hook EFI Security Architectural "
			     L"Protocol\n");

	return EFI_SUCCESS;
}
