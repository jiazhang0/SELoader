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

#include "Internal.h"

#if GNU_EFI_VERSION <= 303
EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
#endif

STATIC EFI_STATUS
LoadImage(CONST CHAR16 *Path, VOID *ImageBuffer, UINTN ImageBufferSize,
	  EFI_HANDLE *ImageHandle)
{
	EFI_LOADED_IMAGE *LoadedImage;
	EFI_STATUS Status;

	Status = EfiProtocolOpen(gThisImage, &gEfiLoadedImageProtocolGuid,
				 (VOID **)&LoadedImage);
	if (EFI_ERROR(Status))
		return Status;

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

	/*
	 * shim may re-install the Sap/Sap2 hooks. If so, we hook them
	 * before calling BS->LoadImage.
	 */
        if (EfiSecurityPolicyMokVerifyProtocolInstalled() == TRUE) {
#ifdef EXPERIMENTAL_BUILD
		SapHook();
#endif
		Sap2Hook();
	}

	EFI_HANDLE LoadedImageHandle;

        Status = gBS->LoadImage(FALSE, gThisImage, DevicePath, ImageBuffer,
				ImageBufferSize, &LoadedImageHandle);
	EfiMemoryFree(DevicePath);
	if (EFI_ERROR(Status)) {
		EfiConsolePrintError(L"Failed to load the image "
				     L"%s (err: 0x%x)\n", Path, Status);
		return Status;
	}

	if (ImageHandle)
		*ImageHandle = LoadedImageHandle;
	else
		gBS->UnloadImage(LoadedImageHandle);

	return EFI_SUCCESS;
}

STATIC EFI_STATUS
ExecuteImage(CONST CHAR16 *Path, VOID *ImageBuffer, UINTN ImageBufferSize,
	     BOOLEAN Unload)
{
	EfiConsolePrintDebug(L"Preparing to start the image %s ...\n",
			     Path);

	EFI_HANDLE ImageHandle;
	EFI_STATUS Status;

	Status = LoadImage(Path, ImageBuffer, ImageBufferSize, &ImageHandle);
	if (EFI_ERROR(Status))
		return Status;

	Status = gBS->StartImage(ImageHandle, NULL, NULL);
	if (!EFI_ERROR(Status))
		EfiConsolePrintDebug(L"The image %s exited\n", Path);
	else
		EfiConsolePrintError(L"Failed to start the image "
				     L"%s (err: 0x%x)\n", Path, Status);

	if (Unload == TRUE)
		gBS->UnloadImage(ImageHandle);

	return Status;
}

EFI_STATUS
EfiImageExecuteDriver(CONST CHAR16 *Path)
{
	if (!Path)
		return EFI_INVALID_PARAMETER;

	return ExecuteImage(Path, NULL, 0, FALSE);
}

EFI_STATUS
EfiImageExecute(CONST CHAR16 *Path)
{
	if (!Path)
		return EFI_INVALID_PARAMETER;

	return ExecuteImage(Path, NULL, 0, TRUE);
}

EFI_STATUS
EfiImageLoad(CONST CHAR16 *Path, VOID *ImageBuffer, UINTN ImageBufferSize)
{
	if (!Path)
		return EFI_INVALID_PARAMETER;

	return LoadImage(Path, ImageBuffer, ImageBufferSize, NULL);
}
