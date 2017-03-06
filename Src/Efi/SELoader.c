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

STATIC EFI_STATUS
LoadConfig(VOID)
{
	VOID *Config;
	UINTN ConfigSize;
	EFI_STATUS Status = EfiFileLoad(L"SELoader.cfg", &Config,
					&ConfigSize);
	
	/*
	 * TODO: parse the configuration to allow to customize the
	 * behavior of SELoader.
	 */

	return Status;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS Status = EfiLibraryInitialize(ImageHandle, SystemTable);
	if (EFI_ERROR(Status))
		return Status;

	LoadConfig();

	EfiConsolePrintLevel Level;
	Status = EfiConsoleGetVerbosity(&Level);
	if (!EFI_ERROR(Status) && Level == EFI_CPL_DEBUG) {
		EfiConsolePrintDebug(L"Preparing to load grubx64.efi ...\n");
		EfiStallSeconds(3);
	}

	Status = EfiImageExecute(L"grubx64.efi");

	EfiConsolePrintDebug(L"grubx64.efi exited with "
			     L"0x%x\n", Status);

	return Status;
}
