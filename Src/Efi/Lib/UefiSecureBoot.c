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

EFI_STATUS
UefiSecureBootDeployedMode(UINT8 *DeployedMode)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*DeployedMode);
	EFI_STATUS Status;

	Status = EfiVariableReadGlobal(L"DeployedMode", &Attributes,
				       (VOID **)&DeployedMode, &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintInfo(L"Unable to get DeployedMode "
				    L"(err: 0x%x)\n", Status);
		return Status;
	}

	Status = EFI_UNSUPPORTED;

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid size of DeployedMode "
				     L"(0x%x)\n", VarSize);
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid attribute of DeployedMode "
				     L"(0x%x)\n", Attributes);
		goto Err;
	}

	if (*DeployedMode != 0 && *DeployedMode != 1) {
		EfiConsolePrintError(L"Invalid value of DeployedMode "
				     L"(0x%x)\n", *DeployedMode);
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
			EfiConsolePrintError(L"DeployedMode is not "
					     L"read-only\n");
			Status = EFI_UNSUPPORTED;
			goto Err;
		}
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteGlobal(L"DeployedMode");

	return Status;
}

EFI_STATUS
UefiSecureBootAuditMode(UINT8 *AuditMode)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*AuditMode);
	EFI_STATUS Status;

	Status = EfiVariableReadGlobal(L"AuditMode", &Attributes,
				       (VOID **)&AuditMode, &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to get AuditMode "
				     L"(err: 0x%x)\n", Status);
		return Status;
	}

	Status = EFI_UNSUPPORTED;

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid size of AuditMode "
				     L"(0x%x)\n", VarSize);
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid attribute of AuditMode "
				     L"(0x%x)\n", Attributes);
		goto Err;
	}

	if (*AuditMode != 0 && *AuditMode != 1) {
		EfiConsolePrintError(L"Invalid value of AuditMode "
				     L"(0x%x)\n", *AuditMode);
		goto Err;
	}

	UINT8 DeployedMode;
	Status = UefiSecureBootDeployedMode(&DeployedMode);
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
			EfiConsolePrintError(L"AuditMode is not read-only\n");
			Status = EFI_UNSUPPORTED;
			goto Err;
		}
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteGlobal(L"AuditMode");

	return Status;
}

EFI_STATUS
UefiSecureBootSetupMode(UINT8 *SetupMode)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*SetupMode);
	EFI_STATUS Status;

	Status = EfiVariableReadGlobal(L"SetupMode", &Attributes,
				       (VOID **)&SetupMode, &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to get SetupMode "
				     L"(err: 0x%x)\n", Status);
		return Status;
	}

	Status = EFI_UNSUPPORTED;

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid size of SetupMode "
				     L"(%d-byte)\n", VarSize);
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid attribute of SetupMode "
				     L"(0x%x)\n", Attributes);
		Status = EFI_UNSUPPORTED;
		goto Err;
	}

	if (*SetupMode != 0 && *SetupMode != 1) {
		EfiConsolePrintError(L"Invalid value of SetupMode "
				     L"(0x%x)\n", *SetupMode);
		goto Err;
	}

	/* Check if SetupMode variable is read-only */
	UINT8 InverseSetupMode;

	InverseSetupMode = !*SetupMode;
	VarSize = sizeof(InverseSetupMode);
	Status = EfiVariableWriteGlobal(L"SetupMode", Attributes,
					&InverseSetupMode,
					VarSize);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintError(L"SetupMode variable is not read-only\n");
		Status = EFI_UNSUPPORTED;
		goto Err;
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteGlobal(L"SetupMode");

	return Status;
}

EFI_STATUS
UefiSecureBootState(UINT8 *SecureBoot)
{
	UINT32 Attributes;
	UINTN VarSize = sizeof(*SecureBoot);
	EFI_STATUS Status;

	Status = EfiVariableReadGlobal(L"SecureBoot", &Attributes,
				       (VOID **)&SecureBoot, &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to get SecureBoot "
				     L"(err: 0x%x)\n", Status);
		return Status;
	}

	Status = EFI_UNSUPPORTED;

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid size of SecureBoot "
				     L"(%d-byte)\n", VarSize);
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid attribute of SecureBoot "
				     L"(0x%x)\n", Attributes);
		goto Err;
	}

	if (*SecureBoot != 0 && *SecureBoot != 1) {
		EfiConsolePrintError(L"Invalid value of SecureBoot "
				     L"(0x%x)\n", *SecureBoot);
		goto Err;
	}

	/* Check if SetupMSecureBootode variable is read-only */
	UINT8 InverseSecureBoot;

	InverseSecureBoot = !*SecureBoot;
	VarSize = sizeof(InverseSecureBoot);
	Status = EfiVariableWriteGlobal(L"SecureBoot", Attributes,
					&InverseSecureBoot,
				  	VarSize);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintError(L"SecureBoot variable is not "
				     L"read-only\n");
		Status = EFI_UNSUPPORTED;
		goto Err;
	}

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteGlobal(L"SecureBoot");

	return Status;
}
