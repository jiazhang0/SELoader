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

EFI_STATUS
EfiProtocolOpen(EFI_HANDLE Handle, CONST EFI_GUID *Protocol, VOID **Interface)
{
	EFI_STATUS Status;

	Status = gBS->HandleProtocol(Handle, (EFI_GUID *)Protocol, Interface);
	if (EFI_ERROR(Status))
		EfiConsolePrintError(L"Failed to open the protocol (err: %d)"
				     L"\n", Status);

	return Status;
}

EFI_STATUS
EfiProtocolLocate(CONST EFI_GUID *Protocol, VOID **Interface)
{
	return gBS->LocateProtocol((EFI_GUID *)Protocol, NULL, Interface);
}

EFI_STATUS
EfiProtocolLocateHandles(CONST EFI_GUID *Protocol, EFI_HANDLE **HandleBuffer,
			 UINTN *Handles)
{
	return gBS->LocateHandleBuffer(ByProtocol, (EFI_GUID *)Protocol, NULL,
				       Handles, HandleBuffer);
}

EFI_STATUS
EfiProtocolInstall(EFI_HANDLE *Handle, EFI_GUID *Protocol, VOID *Interface)
{
	return gBS->InstallProtocolInterface(Handle, Protocol,
					     EFI_NATIVE_INTERFACE, Interface);
}
