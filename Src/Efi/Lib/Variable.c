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
#include <BaseLibrary.h>

#if GNU_EFI_VERSION <= 303
EFI_GUID gEfiGlobalVariableGuid = EFI_GLOBAL_VARIABLE;
#endif

/*
 * Vectorized buffer parameters.
 *
 * Data == NULL: Ignore the content of variable
 * DataSize == NULL: Ignore the size of variable
 * *Data == NULL: Return the buffer of variable
 */
EFI_STATUS
EfiVariableRead(CONST CHAR16 *VariableName, CONST EFI_GUID *VendorGuid,
		UINT32 *Attributes, VOID **Data, UINTN *DataSize)
{
	UINTN BufferSize;
	EFI_STATUS Status;

	if (!DataSize)
		DataSize = &BufferSize;
	else
		BufferSize = *DataSize;

	EFI_GET_VARIABLE GetVariable;

	GetVariable = gRT->GetVariable;
	*DataSize = 0;
	Status = GetVariable((CHAR16 *)VariableName, (EFI_GUID *)VendorGuid,
			     Attributes, DataSize, NULL);
	if (Status != EFI_BUFFER_TOO_SMALL)
		return Status;

	/* The caller doesn't care for the content of the variable */
	if (!Data)
		return EFI_SUCCESS;

	VOID *Buffer;

	if (!*Data || BufferSize < *DataSize) {
		Status = EfiMemoryAllocate(*DataSize, &Buffer);
		if (EFI_ERROR(Status))
			return Status;
	} else
		Buffer = *Data;

	Status = GetVariable((CHAR16 *)VariableName, (EFI_GUID *)VendorGuid,
			     Attributes, DataSize, Buffer);
        if (EFI_ERROR(Status)) {
		if (Buffer != *Data)
			EfiMemoryFree(Buffer);
		return Status;
	}

	if (BufferSize && BufferSize < *DataSize) {
		MemCpy(*Data, Buffer, BufferSize);
		EfiMemoryFree(Buffer);
	} else
		*Data = Buffer;

        return Status;
}

EFI_STATUS
EfiVariableWrite(CONST CHAR16 *VariableName, CONST EFI_GUID *VendorGuid,
		 UINT32 Attributes, VOID *Data, UINTN DataSize)
{
	if (!VariableName || !VendorGuid || !Attributes)
		return EFI_INVALID_PARAMETER;

	/*
	 * Deleting a variable should go EfiVariableDelete().
	 */
	if (!Data || !DataSize)
		return EFI_INVALID_PARAMETER;

	return gRT->SetVariable((CHAR16 *)VariableName, (EFI_GUID *)VendorGuid,
				Attributes, DataSize, Data);
}

EFI_STATUS
EfiVariableDelete(CONST CHAR16 *VariableName, CONST EFI_GUID *VendorGuid)
{
	if (!VariableName || !VendorGuid)
		return EFI_INVALID_PARAMETER;

	UINT32 Attributes;
	EFI_STATUS Status;

	Status = EfiVariableRead(VariableName, VendorGuid, &Attributes,
				 NULL, NULL);
	if (EFI_ERROR(Status))
		return Status;

	return gRT->SetVariable((CHAR16 *)VariableName, (EFI_GUID *)VendorGuid,
				Attributes, 0, NULL);
}
