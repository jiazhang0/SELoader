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
#include <SELoader.h>

#define SELOADER_CONFIGURATION			L"SELoader.conf"

#ifndef SELOADER_CHAINLOADER
#  define SELOADER_CHAINLOADER			L"grub" EFI_ARCH L".efi"
#endif

STATIC EFI_STATUS
LoadConfig(VOID)
{
	EfiConsoleTraceDebug(L"Attempting to load SELoader configuration "
			     SELOADER_CONFIGURATION);

	VOID *Config = NULL;
	UINTN ConfigSize = 0;
	EFI_STATUS Status;

	Status = EfiFileLoad(SELOADER_CONFIGURATION, &Config, &ConfigSize);
	
	/*
	 * TODO: parse the configuration to allow to customize the
	 * behavior of SELoader.
	 */

	EfiConsoleTraceDebug(SELOADER_CONFIGURATION L" parsed (err: 0x%x)",
			     Status);

	return Status;
}

STATIC EFI_STATUS
LaunchLoader(VOID)
{
	EfiConsoleTraceDebug(L"Preparing to load " SELOADER_CHAINLOADER
			     L" ...");

	EFI_STATUS Status;

	Status = EfiImageExecute(SELOADER_CHAINLOADER);

	EfiConsoleTraceFault(SELOADER_CHAINLOADER L" exited with 0x%x\n",
			     Status);

	return Status;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS Status;

	Status = EfiLibraryInitialize(ImageHandle, SystemTable);
	if (EFI_ERROR(Status))
		return Status;

	LoadConfig();

	return LaunchLoader();
}
