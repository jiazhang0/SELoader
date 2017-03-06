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
UefiSecureBootGetDeployedMode(UINT8 *DeployedMode)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*DeployedMode);
	EFI_STATUS Status = EfiVariableReadGlobal(L"DeployedMode", &Attributes,
						  (VOID **)&DeployedMode,
						  &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintInfo(L"Unable to get DeployedMode variable\n");
		if (Status == EFI_NOT_FOUND)
			Status = EFI_UNSUPPORTED;

		return Status;
	}

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid DeployedMode variable size "
				     L"0x%x\n", VarSize);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid DeployedMode variable "
				     L"attribute 0x%x\n", Attributes);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (*DeployedMode != 0 && *DeployedMode != 1) {
		EfiConsolePrintError(L"Invalid DeployedMode variable value "
				     L"0x%x\n", *DeployedMode);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	/* Should be read-only when the variable value is 1 */
	if (DeployedMode) {
		UINT8 InverseDeployedMode;

		InverseDeployedMode = 0;
		VarSize = sizeof(InverseDeployedMode);
		Status = EfiVariableWriteGlobal(L"DeployedMode",
						Attributes,
						&InverseDeployedMode,
					 	VarSize);
		if (!EFI_ERROR(Status)) {
			EfiConsolePrintError(L"DeployedMode variable is not "
					     L"read-only\n");
			Status = EFI_INVALID_PARAMETER;
			goto Err;
		}
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteGlobal(L"DeployedMode");

	return Status;
}

EFI_STATUS
UefiSecureBootGetAuditMode(UINT8 *AuditMode)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*AuditMode);
	EFI_STATUS Status = EfiVariableReadGlobal(L"AuditMode", &Attributes,
						  (VOID **)&AuditMode,
						  &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintInfo(L"Unable to get AuditMode variable\n");
		if (Status == EFI_NOT_FOUND)
			Status = EFI_UNSUPPORTED;

		return Status;
	}

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid AuditMode variable size "
				     L"0x%x\n", VarSize);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid AuditMode variable attribute "
				     L"0x%x\n", Attributes);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (*AuditMode != 0 && *AuditMode != 1) {
		EfiConsolePrintError(L"Invalid AuditMode variable value "
				     L"0x%x\n", *AuditMode);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	UINT8 DeployedMode;
	Status = UefiSecureBootGetDeployedMode(&DeployedMode);
	if (EFI_ERROR(Status)) 
		goto Err;

	/* Should be read-only when DeployedMode variable is 1 */
	if (DeployedMode) {
		UINT8 InverseAuditMode;

		InverseAuditMode = 0;
		VarSize = sizeof(InverseAuditMode);
		Status = EfiVariableWriteGlobal(L"AuditMode", Attributes,
						&InverseAuditMode,
						VarSize);
		if (!EFI_ERROR(Status)) {
			EfiConsolePrintError(L"AuditMode variable is not "
					     L"read-only\n");
			Status = EFI_INVALID_PARAMETER;
			goto Err;
		}
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteGlobal(L"AuditMode");

	return Status;
}

EFI_STATUS
UefiSecureBootGetSetupMode(UINT8 *SetupMode)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*SetupMode);
	EFI_STATUS Status = EfiVariableReadGlobal(L"SetupMode", &Attributes,
						  (VOID **)&SetupMode,
						  &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintInfo(L"UEFI Secure Boot is not supported "
				    L"by the platform firmware\n");
		if (Status == EFI_NOT_FOUND)
			Status = EFI_UNSUPPORTED;
		else
			goto Err;

		return Status;
	}

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid SetupMode variable size "
				     L"%d-byte\n", VarSize);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid SetupMode variable attribute "
				     L"0x%x\n", Attributes);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (*SetupMode != 0 && *SetupMode != 1) {
		EfiConsolePrintError(L"Invalid SetupMode variable value "
				     L"0x%x\n", *SetupMode);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	/* Check if SetupMode variable is read-only */
	UINT8 InverseSetupMode = !*SetupMode;
	VarSize = sizeof(InverseSetupMode);
	Status = EfiVariableWriteGlobal(L"SetupMode", Attributes,
					&InverseSetupMode,
					VarSize);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintError(L"SetupMode variable is not read-only\n");
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteGlobal(L"SetupMode");

	return Status;
}

EFI_STATUS
UefiSecureBootGetStatus(UINT8 *SecureBoot)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*SecureBoot);
	EFI_STATUS Status = EfiVariableReadGlobal(L"SecureBoot", &Attributes,
						  (VOID **)&SecureBoot,
						  &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintInfo(L"UEFI Secure Boot is not supported "
				    L"by the platform firmware\n");
		if (Status == EFI_NOT_FOUND)
			Status = EFI_UNSUPPORTED;
		else
			goto Err;

		return Status;
	}

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid SecureBoot variable size "
				     L"%d-byte\n", VarSize);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid SecureBoot variable attribute "
				     L"0x%x\n", Attributes);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (*SecureBoot != 0 && *SecureBoot != 1) {
		EfiConsolePrintError(L"Invalid SecureBoot variable value "
				     L"0x%x\n", *SecureBoot);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	/* Check if SetupMSecureBootode variable is read-only */
	UINT8 InverseSecureBoot = !*SecureBoot;
	VarSize = sizeof(InverseSecureBoot);
	Status = EfiVariableWriteGlobal(L"SecureBoot", Attributes,
					&InverseSecureBoot,
				  	VarSize);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintError(L"SecureBoot variable is not read-only\n");
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteGlobal(L"SecureBoot");

	return Status;
}
