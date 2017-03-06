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
STATIC BOOLEAN MokSecureBootEnabled = FALSE;
STATIC BOOLEAN SelSecureBootEnabled = FALSE;

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

	EfiConsolePrintInfo(L"SELoader Secure Boot Mode: %s\n",
			    SelSecureBootEnabled ? L"Enabled" :
						   L"Disabled");
}

STATIC EFI_STATUS
InitializeSecurityPolicy(VOID)
{
	UINT8 SetupMode;
	EFI_STATUS Status = UefiSecureBootGetSetupMode(&SetupMode);
	if (EFI_ERROR(Status))
		return Status;

	EfiConsolePrintDebug(L"Platform firmware is in %s mode\n",
			     SetupMode ? L"Setup" : L"User");

	UINT8 SecureBoot;
	Status = UefiSecureBootGetStatus(&SecureBoot);
	if (EFI_ERROR(Status))
		return Status;

	EfiConsolePrintDebug(L"Platform firmware is %soperating in Secure "
			     L"Boot mode\n", SecureBoot ? L"" : L"not ");

	if (SetupMode && SecureBoot) {
		EfiConsolePrintError(L"Platform firmware is in Setup mode but "
				     L"SecureBoot is enabled\n");
		return EFI_INVALID_PARAMETER;
	}

	/* TODO: support MOK secure boot */
	UINT8 MokSecureBoot = 0;

	UINT8 SelSecureBoot = 0;
	Status = SelSecureBootMode(&SelSecureBoot);
	if (EFI_ERROR(Status)) {
		if (Status != EFI_NOT_FOUND)
			return Status;
	}

	if (!SetupMode)
		UefiSecureBootProvisioned = TRUE;

	if (SecureBoot == 1)
		UefiSecureBootEnabled = TRUE;

	if (MokSecureBoot == 1)
		MokSecureBootEnabled = TRUE;

	/*
	 * Enable SEL Secure Boot as long as UEFI Secure Boot already
	 * enabled.
	 */
	if (UefiSecureBootEnabled == TRUE) {
		SelSecureBootEnabled = TRUE;

		if (SelSecureBoot == 0) {	
			UINT32 Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS;
			SelSecureBoot = 1;
			Status = EfiVariableWriteSel(L"SelSecureBoot",
						     Attributes,
						     (VOID *)&SelSecureBoot,
						     sizeof(SelSecureBoot));
			if (EFI_ERROR(Status))
				SelSecureBootEnabled = FALSE;
		}
	}

	SecurityPolicyInitialized = TRUE;

	return EFI_SUCCESS;
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

	if (SecurityPolicyInitialized == FALSE) {
		Status = InitializeSecurityPolicy();
		if (EFI_ERROR(Status))
			return Status;
	}

	EFI_SIGNATURE_LIST *Data = NULL;
	UINTN DataSize = 0;
	Status = EFI_INVALID_PARAMETER;
	BOOLEAN Ignored = FALSE;

	if (!StrCmp(Name, L"db") || !StrCmp(Name, L"dbx")) {
		if (UefiSecureBootEnabled == TRUE)
			Status = EfiVariableReadSecure(Name, NULL,
						       (VOID **)&Data,
						       &DataSize);
		else
			Ignored = TRUE;
	} else if (!StrCmp(Name, L"MokList") || !StrCmp(Name, L"MokListX")) {
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
	} else
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
	if (SecurityPolicyInitialized == FALSE) {
		EFI_STATUS Status = InitializeSecurityPolicy();
		if (EFI_ERROR(Status))
			return;
	}

	PrintSecurityPolicy();
}
