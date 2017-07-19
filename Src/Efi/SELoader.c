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
#include <SELoader.h>

#define SELOADER_CONFIGURATION			L"SELoader.conf"

#ifndef SELOADER_CHAINLOADER
#  define SELOADER_CHAINLOADER			L"grub" EFI_ARCH L".efi"
#endif

#ifdef EXPERIMENTAL_BUILD
STATIC EFI_STATUS
LoadConfig(VOID)
{
	EfiConsoleTraceDebug(L"Attempting to load SELoader configuration "
			     SELOADER_CONFIGURATION L" ...\n");

	BOOLEAN ParseContent = FALSE;
	EFI_STATUS Status;

	if (ParseContent == FALSE)
		Status = EfiFileLoad(SELOADER_CONFIGURATION, NULL, 0);
	else {
		VOID *Config = NULL;
		UINTN ConfigSize = 0;

		Status = EfiFileLoad(SELOADER_CONFIGURATION, &Config,
				     &ConfigSize);
	}

	/*
	 * TODO: parse the configuration to allow to customize the
	 * behavior of SELoader.
	 */

	EfiConsoleTraceDebug(SELOADER_CONFIGURATION L" parsed (err: 0x%x)\n",
			     Status);

	return Status;
}
#else
STATIC EFI_STATUS
LoadConfig(VOID)
{
	return EFI_UNSUPPORTED;
}
#endif

STATIC EFI_STATUS
LaunchLoader(VOID)
{
	EfiConsoleTraceInfo(L"Preparing to load " SELOADER_CHAINLOADER
			    L" ...\n");

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
