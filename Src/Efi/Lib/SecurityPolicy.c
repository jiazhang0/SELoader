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
#include <BaseLibrary.h>

#include "Internal.h"

EFI_GUID gEfiImageSecurityDatabaseGuid = EFI_IMAGE_SECURITY_DATABASE_GUID;

STATIC BOOLEAN SecurityPolicyInitialized = FALSE;
STATIC BOOLEAN UefiSecureBootProvisioned = FALSE;
STATIC BOOLEAN UefiSecureBootEnabled = FALSE;
STATIC BOOLEAN MokSecureBootProvisioned = FALSE;
STATIC BOOLEAN MokSecureBootEnabled = FALSE;

STATIC VOID
PrintSecurityPolicy(VOID)
{	
	EfiConsolePrintInfo(L"UEFI Secure Boot Mode: %s\n",
			    UefiSecureBootEnabled ? L"Enabled" :
						    L"Disabled");

	if (UefiSecureBootEnabled == FALSE)
		EfiConsolePrintInfo(L"Setup Mode: %s\n",
				    UefiSecureBootProvisioned ? L"No" :
								L"Yes");

	EfiConsolePrintInfo(L"MOK Secure Boot Mode: %s\n",
			    MokSecureBootEnabled ? L"Enabled" :
						   L"Disabled");

	if (MokSecureBootEnabled == FALSE)
		EfiConsolePrintInfo(L"MOK Verify Protocol installed: %s\n",
				    MokSecureBootProvisioned ? L"Yes" :
							       L"No");
}

STATIC VOID
InitializeSecurityPolicy(VOID)
{
	UINT8 SetupMode = 1;
	EFI_STATUS Status;

	Status = UefiSecureBootSetupMode(&SetupMode);
	if (!EFI_ERROR(Status))
		EfiConsolePrintDebug(L"Platform firmware is in %s mode\n",
				     SetupMode == 1 ? L"Setup" : L"User");

	UINT8 SecureBoot = 0;

	Status = UefiSecureBootState(&SecureBoot);
	if (!EFI_ERROR(Status))
		EfiConsolePrintDebug(L"Platform firmware is %soperating in "
				     L"Secure Boot mode\n", SecureBoot == 1 ?
							    L"" : L"not ");

	if (SetupMode == 1 && SecureBoot == 1) {
		EfiConsolePrintError(L"Platform firmware is in Setup mode but "
				     L"SecureBoot is enabled\n");
		SecureBoot = 0;
	}

	BOOLEAN MokVerifyInstalled = FALSE;

	Status = MokVerifyProtocolInstalled(&MokVerifyInstalled);
	if (!EFI_ERROR(Status) && MokVerifyInstalled == TRUE)
		EfiConsolePrintDebug(L"MOK Verify Protocol installed\n");

	UINT8 MokSBState = 1;

	Status = MokSecureBootState(&MokSBState);
	if (!EFI_ERROR(Status))
		EfiConsolePrintDebug(L"Shim loader is %soperating in "
				     L"MOK Secure Boot mode\n", !MokSBState ?
								L"" : L"not ");

	if (MokVerifyInstalled == FALSE && MokSBState == 0) {
		EfiConsolePrintError(L"Shim loader is in MOK Secure Boot mode "
				     L"but MOK Verify Protocol not "
				     L"installed\n");
		MokSBState = 1;
	}

	if (SetupMode == 0)
		UefiSecureBootProvisioned = TRUE;

	if (MokVerifyInstalled == TRUE)
		MokSecureBootProvisioned = TRUE;

	if (UefiSecureBootProvisioned == TRUE && SecureBoot == 1) {
		UefiSecureBootEnabled = TRUE;

		HookSap();
		HookSap2();

		if (MokSecureBootProvisioned == TRUE && MokSBState == 0)
			MokSecureBootEnabled = TRUE;
	}

	/*
	 * Install MOK2 Verify Protocol as long as UEFI/MOK Secure Boot already
	 * enabled.
	 */
	if (UefiSecureBootEnabled == TRUE &&
	    (MokSecureBootProvisioned == FALSE ||
	     MokSecureBootEnabled == TRUE)) {
		EfiConsolePrintDebug(L"Attempting to initialize MOK2 Verify "
				     L"Protocol ...\n");

		Status = Mok2VerifyInitialize();
		if (EFI_ERROR(Status)) {
			MokSecureBootEnabled = FALSE;
			EfiConsolePrintError(L"Assuming MOK Secure Boot "
					     L"disabled due to error on "
					     L"installing MOK2 Verify "
					     L"Protocol\n");
		}
	} else
		EfiConsolePrintInfo(L"Ignore to install MOK2 Verify "
				    L"Protocol\n");

	SecurityPolicyInitialized = TRUE;
}

EFI_STATUS
EfiSecurityPolicyLoad(CONST CHAR16 *Name, EFI_SIGNATURE_LIST **SignatureList,
		      UINTN *SignatureListSize)
{
	if (!Name)
		return EFI_INVALID_PARAMETER;

	if (SignatureList)
		*SignatureList = NULL;

	if (SignatureListSize)
		*SignatureListSize = 0;

	EFI_STATUS Status;

	if (SecurityPolicyInitialized == FALSE)
		InitializeSecurityPolicy();

	EFI_SIGNATURE_LIST *Data = NULL;
	UINTN DataSize = 0;
	BOOLEAN Ignored = FALSE;

	Status = EFI_INVALID_PARAMETER;

	if (!StrCmp(Name, EFI_IMAGE_SECURITY_DATABASE) ||
	    !StrCmp(Name, EFI_IMAGE_SECURITY_DATABASE1)) {
		if (UefiSecureBootEnabled == TRUE)
			Status = EfiVariableReadSecure(Name, NULL,
						       (VOID **)&Data,
						       &DataSize);
		else
			Ignored = TRUE;
	} else if (!StrCmp(Name, L"MokListRT") || !StrCmp(Name, L"MokListXRT")) {
		if (UefiSecureBootEnabled == TRUE &&
		    MokSecureBootEnabled == TRUE)
			Status = EfiVariableReadMok(Name, NULL,
						    (VOID **)&Data,
						    &DataSize);
		else
			Ignored = TRUE;
	} else {
		EfiConsolePrintError(L"Invalid security policy object %s "
				     L"specified\n", Name);
		return Status;
	}

	if (Ignored == TRUE) {
		EfiConsolePrintDebug(L"Ignore loading the security policy "
				     L"object %s ignored\n", Name);
		return EFI_SUCCESS;
	}

	if (!EFI_ERROR(Status)) {
		EfiConsolePrintDebug(L"The security policy object %s "
				     L"loaded\n", Name);

		if (SignatureList)
			*SignatureList = Data;

		if (SignatureListSize)
			*SignatureListSize = DataSize;
	} else if (Status == EFI_NOT_FOUND)
		EfiConsolePrintDebug(L"The security policy object %s is "
				     L"empty\n", Name);
	else
		EfiConsolePrintError(L"Failed to load the security policy "
				     L"object %s (err: 0x%x)\n",
				     Name, Status);

	return Status;
}

EFI_STATUS
EfiSecurityPolicyFree(EFI_SIGNATURE_LIST **SignatureList)
{
	return EFI_SUCCESS;
}

VOID
EfiSecurityPolicyPrint(VOID)
{
	if (SecurityPolicyInitialized == FALSE)
		InitializeSecurityPolicy();

	PrintSecurityPolicy();
}

BOOLEAN
EfiSecurityPolicySecureBootEnabled(VOID)
{
	if (SecurityPolicyInitialized == FALSE)
		InitializeSecurityPolicy();

	BOOLEAN Result = FALSE;

	if (UefiSecureBootEnabled == TRUE) {
		if (MokSecureBootProvisioned == TRUE) {
			if (MokSecureBootEnabled == TRUE)
				Result = TRUE;
		} else
			Result = TRUE;
	}

	return Result;
}

VOID
SecurityPolicyInitialize(VOID)
{
	InitializeSecurityPolicy();
}
