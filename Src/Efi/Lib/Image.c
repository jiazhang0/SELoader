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

#if GNU_EFI_VERSION <= 303
EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
#endif

STATIC EFI_STATUS
Execute(CONST CHAR16 *Path, VOID *ImageBuffer, UINTN ImageBufferSize,
	BOOLEAN Unload)
{
	EFI_LOADED_IMAGE *LoadedImage;
	EFI_STATUS Status = EfiProtocolOpen(gThisImage,
					    &gEfiLoadedImageProtocolGuid,
					    (VOID **)&LoadedImage);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to locate parent device "
				     L"handle (err: 0x%x)\n", Status);
		return Status;
	}

	CHAR16 *FilePath;
	Status = EfiDevicePathCreate(Path, &FilePath);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to create the file path for "
				     L"%s (err: 0x%x)\n", Path, Status);
		return Status;
	}

	EFI_DEVICE_PATH *DevicePath = FileDevicePath(LoadedImage->DeviceHandle,
						     FilePath);
	EfiMemoryFree(FilePath);
	if (!DevicePath) {
		EfiConsolePrintError(L"Failed to create the device path for "
				     L"%s (err: 0x%x)\n", Path, Status);
		return Status;
	}	

	EFI_HANDLE ImageHandle;
        Status = gBS->LoadImage(FALSE, gThisImage, DevicePath, ImageBuffer,
				ImageBufferSize, &ImageHandle);
	EfiMemoryFree(DevicePath);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to load the image "
				     L"%s (err: 0x%x)\n", Path, Status);
		return Status;
	}

	EfiConsolePrintDebug(L"Preparing to start the image %s ...\n",
			     Path);

	Status = gBS->StartImage(ImageHandle, NULL, NULL);
	if (!EFI_ERROR(Status))
		EfiConsolePrintDebug(L"Image %s exited\n", Path);
	else
		EfiConsolePrintError(L"Failed to start the image "
				     L"%s (err: 0x%x)\n", Path, Status);

	if (Unload == TRUE)
		gBS->UnloadImage(ImageHandle);

	return Status;
}

EFI_STATUS
EfiImageLoadDriver(CONST CHAR16 *Path)
{
	return Execute(Path, NULL, 0, FALSE);
}

EFI_STATUS
EfiImageExecute(CONST CHAR16 *Path)
{
	return Execute(Path, NULL, 0, TRUE);
}

EFI_STATUS
EfiImageExecuteSecure(CONST CHAR16 *Path)
{
	CHAR16 *SignaturePath = StrAppend(Path, L".p7s");
	if (!SignaturePath) {
		EfiConsolePrintError(L"Failed to set the path for signature "
				     L"file %s\n", Path);
		return EFI_OUT_OF_RESOURCES;
	}

	VOID *Signature;
	UINTN SignatureSize;
	EFI_STATUS Status = EfiFileLoad(SignaturePath, &Signature,
					&SignatureSize);
	EfiMemoryFree(SignaturePath);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to load the signature file %s\n",
				     SignaturePath);
		return Status;
	}

	EfiConsolePrintDebug(L"The signature file %s loaded\n",
			     SignaturePath);

        VOID *Data;
        UINTN DataSize;
	Status = EfiFileLoad(Path, &Data, &DataSize);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to load the file %s\n",
				     Path);
		EfiMemoryFree(Signature);
		return Status;
	}

	EfiConsolePrintDebug(L"The signed data file %s loaded\n",
			     Path);

	Status = EfiSignatureVerifyBuffer(Signature, SignatureSize, Data,
					  DataSize);
	EfiMemoryFree(Signature);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to verify the file %s\n",
				     Path);
		EfiMemoryFree(Data);
		return Status;
	}

	EfiConsolePrintDebug(L"Attempting to load the image %s ...\n", Path);

	return Execute(Path, Data, DataSize, TRUE);
}
