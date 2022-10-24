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
#include <BaseLibrary.h>

#include "Internal.h"

EFI_GUID gEfiImageSecurityDatabaseGuid = EFI_IMAGE_SECURITY_DATABASE_GUID;

STATIC BOOLEAN SecurityPolicyInitialized = FALSE;
STATIC BOOLEAN UefiSecureBootProvisioned = FALSE;
STATIC BOOLEAN UefiSecureBootEnabled = FALSE;
STATIC BOOLEAN MokSecureBootProvisioned = FALSE;
STATIC BOOLEAN MokSecureBootEnabled = FALSE;
STATIC BOOLEAN MokSecureBootUnavailable = FALSE;

STATIC VOID
PrintSecurityPolicy(VOID)
{	
	EfiConsolePrintInfo(L"UEFI Secure Boot Mode: %s\n",
			    UefiSecureBootEnabled ? L"Enabled" :
						    L"Disabled");

	EfiConsolePrintInfo(L"Setup Mode: %s\n",
			    UefiSecureBootProvisioned ? L"No" : L"Yes");

	EfiConsolePrintInfo(L"MOK Verify Protocol installed: %s\n",
			    MokSecureBootProvisioned ? L"Yes" : L"No");

	EfiConsolePrintInfo(L"MOK Secure Boot Mode: %s\n",
			    MokSecureBootEnabled ? L"Enabled" :
						   L"Disabled");
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

	if (SetupMode == 0)
		UefiSecureBootProvisioned = TRUE;

	UINT8 SecureBoot = 0;

	Status = UefiSecureBootState(&SecureBoot);
	if (!EFI_ERROR(Status))
		EfiConsolePrintDebug(L"Platform firmware is %soperating in "
				     L"Secure Boot mode\n", SecureBoot == 1 ?
							    L"" : L"not ");

	if (SecureBoot == 1)
		UefiSecureBootEnabled = TRUE;

	if (UefiSecureBootProvisioned == FALSE &&
	    UefiSecureBootEnabled == TRUE)
		EfiConsolePrintFault(L"Platform firmware is in Setup mode but "
				     L"SecureBoot is enabled\n");

	BOOLEAN MokVerifyInstalled = FALSE;

	Status = MokVerifyProtocolInstalled(&MokVerifyInstalled);
	if (EFI_ERROR(Status))
		EfiConsolePrintFault(L"Unable to know whether MOK Verify "
				     L"Protocol is installed\n");

	EfiConsolePrintDebug(L"MOK Verify Protocol is %sinstalled\n",
			     MokVerifyInstalled == FALSE ? L"not " : L"");

	if (MokVerifyInstalled == TRUE)
		MokSecureBootProvisioned = TRUE;

	/* MokSBState == 1 denotes MOK Secure Boot is disabled */
	UINT8 MokSBState = 1;

	Status = MokSecureBootState(&MokSBState);
	if (EFI_ERROR(Status))
		EfiConsolePrintFault(L"Unable to retrieve MokSBState\n");

	/*
	 * MokSBState == 2 is not a valid value according to MOK Secure
	 * Boot spec. But we use it to indicate the situation where
	 * MokSBState is undefined.
	 */
	if (!MokSBState || MokSBState == 2)
		MokSecureBootEnabled = TRUE;

	if (MokSecureBootProvisioned == TRUE && MokSecureBootEnabled == FALSE)
		EfiConsolePrintFault(L"Shim loader is not in MOK Secure Boot "
				     L"mode but MOK Verify Protocol installed"
				     L"\n");
	else if (MokSecureBootProvisioned == FALSE &&
		 MokSecureBootEnabled == TRUE) {
		/*
		 * If boot manager boots up SELoader directly without shim,
		 * this case will happen. For the case of MokSBState == 0,
		 * it must be the case where fallback is used.
		 */
		if (MokSBState == 2) {
			MokSecureBootUnavailable = TRUE;
			MokSecureBootEnabled = FALSE;
		}
	}

	if (MokSecureBootUnavailable == FALSE) {
		if (MokSecureBootProvisioned == TRUE)
			EfiConsolePrintDebug(L"Shim loader is %soperating in "
					     L"MOK Secure Boot mode\n",
					     MokSecureBootEnabled == TRUE ?
					     L"" : L"not ");
		else
			EfiConsolePrintDebug(L"MOK Verify Protocol was "
					     L"temporarily uninstalled\n");
	} else
		EfiConsolePrintDebug(L"Shim loader is not used\n");

	if (UefiSecureBootEnabled == TRUE) {
		/*
		 * If MOK Secure Boot is disabled, we still need the hooks
		 * to make sure the authentication check derived from UEFI
		 * Secure Boot is ignored.
		 */
#ifdef EXPERIMENTAL_BUILD
		SapHook();
#endif
		Sap2Hook();

		if (MokSecureBootProvisioned == TRUE) {
			Status = MokVerifyProtocolHook();

			if (EFI_ERROR(Status))
				EfiConsolePrintFault(L"Unable to hook MOK "
						     L"Verify Protocol\n");
		}

		EfiConsolePrintDebug(L"Attempting to initialize MOK2 "
				     L"Verify Protocol ...\n");

		Status = Mok2VerifyInitialize();
		if (EFI_ERROR(Status))
			EfiConsolePrintFault(L"Unable to initialize MOK2 "
					     L"Verify Protocol\n");
	}

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
	} else if (!StrCmp(Name, L"MokListRT") ||
		   !StrCmp(Name, L"MokList") ||
		   !StrCmp(Name, L"MokListX") ||
		   !StrCmp(Name, L"MokListXRT")) {
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
				     L"object %s\n", Name);
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

	return UefiSecureBootEnabled == TRUE &&
	       (MokSecureBootEnabled == TRUE ||
		(MokSecureBootUnavailable == TRUE));
}

BOOLEAN
EfiSecurityPolicyMokVerifyProtocolInstalled(VOID)
{
	if (SecurityPolicyInitialized == FALSE)
		InitializeSecurityPolicy();

	return EfiSecurityPolicySecureBootEnabled() == TRUE &&
	       MokSecureBootProvisioned == TRUE;
}

VOID
SecurityPolicyInitialize(VOID)
{
	InitializeSecurityPolicy();
}
