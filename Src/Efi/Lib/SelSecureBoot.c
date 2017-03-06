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

#include "Internal.h"

EFI_GUID gSelSecureBootProtocolGuid = SELOADER_SECURE_BOOT_PROTOCOL_GUID;

EFI_STATUS
SelSecureBootMode(UINT8 *SelSecureBoot)
{
	if (!SelSecureBoot)
		return EFI_INVALID_PARAMETER;

	UINT32 Attributes;
	UINTN VarSize = sizeof(*SelSecureBoot);
	EFI_STATUS Status = EfiVariableReadSel(L"SelSecureBoot", &Attributes,
					       (VOID **)&SelSecureBoot,
					       &VarSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintInfo(L"Unable to get SelSecureBoot "
				    L"variable (err: 0x%x)\n", Status);
		return Status;
	}

	if (VarSize != 1) {
		EfiConsolePrintError(L"Invalid SelSecureBoot variable size "
				     L"0x%x\n", VarSize);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS |
			   EFI_VARIABLE_RUNTIME_ACCESS)) {
		EfiConsolePrintError(L"Invalid SelSecureBoot variable "
				     L"attribute 0x%x\n", Attributes);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	if (*SelSecureBoot != 0 && *SelSecureBoot != 1) {
		EfiConsolePrintError(L"Invalid SelSecureBoot variable value "
				     L"0x%x\n", *SelSecureBoot);
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}

	/* FIXME: Should be read-only when the variable value is 1 */
#if 0
	UINT8 InverseSelSecureBoot = !*SelSecureBoot;
	VarSize = sizeof(InverseSelSecureBoot);
	Status = EfiVariableWriteSel(L"SelSecureBoot", Attributes,
				     &InverseSelSecureBoot, VarSize);
	if (!EFI_ERROR(Status)) {
		EfiConsolePrintError(L"SelSecureBoot variable is not "
				     L"read-only\n");
		Status = EFI_INVALID_PARAMETER;
		goto Err;
	}
#endif

	return EFI_SUCCESS;

Err:
	EfiVariableDeleteSel(L"SelSecureBoot");

	return Status;
}

EFI_STATUS
SelSecureBootVerifyBuffer(UINT8 *Data, UINTN DataSize,
			  UINT8 *Signature, UINTN SignatureSize)
{
	return Pkcs7Verify(Data, DataSize, Signature, SignatureSize);
}
