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

#ifndef GNU_EFI
EFI_GUID gEfiSimpleFileSystemProtocolGuid =
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
#endif

STATIC EFI_STATUS
LoadFile(CONST CHAR16 *Path, VOID **Data, UINTN *DataSize, BOOLEAN Verify);

STATIC EFI_STATUS
VerifyFileBuffer(VOID *Buffer, UINTN BufferSize, CONST CHAR16 *Path)
{
	UINT8 SelSecureBoot;
	EFI_STATUS Status = SelSecureBootMode(&SelSecureBoot);
	if (EFI_ERROR(Status)) {
		if (Status == EFI_NOT_FOUND)
			EfiConsolePrintDebug(L"Ignore to verify file %s "
					     L"due to SelSecureBoot unset\n",
					     Path);
		return EFI_SUCCESS;
	}

	if (SelSecureBoot == 0) {
		EfiConsolePrintDebug(L"Ignore to verify file %s\n",
				     Path);
		return EFI_SUCCESS;
	}

	CHAR16 *SignaturePath = StrAppend(Path, L".p7b");
	if (!SignaturePath)
		return EFI_OUT_OF_RESOURCES;

	VOID *Signature;
	UINTN SignatureSize;
	Status = LoadFile(SignaturePath, &Signature, &SignatureSize, FALSE);
	EfiMemoryFree(SignaturePath);
	if (EFI_ERROR(Status))
		return Status;

	Status = SelSecureBootVerifyBuffer(Buffer, BufferSize,
					   Signature, SignatureSize);
	EfiMemoryFree(Signature);
	if (!EFI_ERROR(Status))
		EfiConsolePrintError(L"Succeeded to verify %s\n",
				     Path);
	else
		EfiConsolePrintDebug(L"Failed to verify the file %s "
				     L"(err: 0x%x)\n", Path,
				     Status);

	return Status;
}

STATIC EFI_STATUS
LoadFile(CONST CHAR16 *Path, VOID **Data, UINTN *DataSize, BOOLEAN Verify)
{
	EFI_FILE_IO_INTERFACE *FileSystem;
	EFI_STATUS Status = EfiProtocolOpen(EfiContext->LoaderDevice,
					    &gEfiSimpleFileSystemProtocolGuid,
					    (VOID **)&FileSystem);
	if (EFI_ERROR(Status))
		return Status;

	EFI_FILE_HANDLE Root;
	Status = FileSystem->OpenVolume(FileSystem, &Root);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to open volume 0x%x for %s\n",
				     Status, Path);
		return Status;
	}

	CHAR16 *FilePath;
	Status = EfiDevicePathCreate(Path, &FilePath);
	if (EFI_ERROR(Status)) {
		Root->Close(Root);
		EfiConsolePrintError(L"Unable to create the file path "
				     L"for %s (err: 0x%x)\n", Path, Status);
		return Status;
	}

	EFI_FILE_HANDLE FileHandle;
	Status = Root->Open(Root, &FileHandle, FilePath, EFI_FILE_MODE_READ,
			    0);
	Root->Close(Root);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Unable to open file %s for read: "
				     L"0x%x\n", FilePath, Status);
		EfiMemoryFree(FilePath);
		return Status;
	}

	EFI_FILE_INFO *FileInfo;
	FileInfo = LibFileInfo(FileHandle);
	if (!FileInfo) {
		EfiConsolePrintError(L"Unable to get file info for %s\n",
				     FilePath);
		EfiMemoryFree(FilePath);
		return EFI_OUT_OF_RESOURCES;
	}

	UINTN FileSize = (UINTN)FileInfo->FileSize;
	EfiMemoryFree(FileInfo);
	if (!FileSize) {
		EfiConsolePrintError(L"Empty file %s\n", FilePath);
		EfiMemoryFree(FilePath);
		return EFI_INVALID_PARAMETER;
	}

	VOID *FileBuffer;
	Status = EfiMemoryAllocate(FileSize, &FileBuffer);
	if (!FileBuffer) {
		EfiConsolePrintError(L"Failed to allocate memory for %s\n",
				     FilePath);
		EfiMemoryFree(FilePath);
		return EFI_OUT_OF_RESOURCES;
	}

	Status = FileHandle->Read(FileHandle, &FileSize, FileBuffer);
	FileHandle->Close(FileHandle);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to read file %s: 0x%x\n",
				     FilePath, Status);
		EfiMemoryFree(FileBuffer);
		EfiMemoryFree(FilePath);
		return Status;
	}

	if (Verify == TRUE) {
		Status = VerifyFileBuffer(FileBuffer, FileSize, FilePath);
		if (EFI_ERROR(Status)) {	
			EfiMemoryFree(FileBuffer);
			EfiMemoryFree(FilePath);
			return Status;
		}
	}

	if (Data)
		*Data = FileBuffer;

	if (DataSize)
		*DataSize = FileSize;

	EfiConsolePrintDebug(L"File %s loaded (%d-byte)\n", FilePath,
			     FileSize);

	EfiMemoryFree(FilePath);

	return EFI_SUCCESS;
}

EFI_STATUS
EfiFileLoad(CONST CHAR16 *Path, VOID **Data, UINTN *DataSize)
{
	return LoadFile(Path, Data, DataSize, TRUE);
}
