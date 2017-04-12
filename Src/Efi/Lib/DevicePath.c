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

#if 0
STATIC EFI_STATUS
BaseName(CONST CHAR16 *Path, CHAR16 **Name)
{
	if (!Path || !Name)
		return EFI_INVALID_PARAMETER;

	CONST CHAR16 *SubPath = StrrChr(Path, L'\\');
	if (SubPath)
		++SubPath;
	else
		SubPath = Path;

	*Name = StrDup(SubPath);

	return EFI_SUCCESS;
}
#endif

STATIC EFI_STATUS
DirectoryName(CONST CHAR16 *Path, CHAR16 **Name)
{
	if (!Path || !Name)
		return EFI_INVALID_PARAMETER;

	CONST CHAR16 *SubPath = StrrChr(Path, L'\\');
	if (!SubPath)
		SubPath = Path;

	UINTN Length = SubPath - Path + 1;
	EFI_STATUS Status = EfiMemoryAllocate(Length * sizeof(CHAR16),
					      (VOID **)Name);
	if (EFI_ERROR(Status))
		return Status;

	StrnCpy(*Name, Path, Length);
	(*Name)[Length - 1] = L'\0';

	return EFI_SUCCESS;
}

STATIC EFI_STATUS
AppendFilePath(CONST CHAR16 *Path, CHAR16 **FilePath)
{
	if (!Path || !FilePath)
		return EFI_INVALID_PARAMETER;

	CHAR16 *DirectoryPath;
	EFI_STATUS Status = EfiDevicePathRootDirectory(&DirectoryPath);
	if (EFI_ERROR(Status))
		return Status;

	CHAR16 *FilePathPrefix = StrAppend(DirectoryPath, L"\\");
	EfiMemoryFree(DirectoryPath);
	if (!FilePathPrefix)
		return Status;

	*FilePath = StrAppend(FilePathPrefix, Path);
	EfiMemoryFree(FilePathPrefix);

	return EFI_SUCCESS;
}

STATIC VOID
FixPath(CHAR16 *Path)
{
	UINTN Index = 0;

	while (Path[Index]) {
		if (Path[Index] == L'/')
			Path[Index] = L'\\';

		if (Path[Index + 1] == L'/')
			Path[Index + 1] = L'\\';

		if (Path[Index] != L'\\' || Path[Index + 1] != L'\\') {
			++Index;
			continue;
		}

		UINTN NextIndex = Index;

		do
			Path[NextIndex] = Path[NextIndex + 1];
		while (Path[++NextIndex]);
	}
}

EFI_STATUS
EfiDevicePathRootDirectory(CHAR16 **DirectoryPath)
{
	if (!DirectoryPath)
		return EFI_INVALID_PARAMETER;

	EFI_LOADED_IMAGE *LoadedImage;
	EFI_STATUS Status = EfiProtocolOpen(gThisImage,
					    &gEfiLoadedImageProtocolGuid,
					    (VOID **)&LoadedImage);
	if (EFI_ERROR(Status))
		return Status;

	CHAR16 *LoaderFilePath = DevicePathToStr(LoadedImage->FilePath);
	if (!LoaderFilePath)
		return EFI_OUT_OF_RESOURCES;

	FixPath(LoaderFilePath);

	Status = DirectoryName(LoaderFilePath, DirectoryPath);
	EfiMemoryFree(LoaderFilePath);

	return Status;
}

EFI_STATUS
EfiDevicePathCreate(CONST CHAR16 *Path, CHAR16 **FilePath)
{
	if (!Path || !FilePath)
		return EFI_INVALID_PARAMETER;

	EFI_STATUS Status = EFI_OUT_OF_RESOURCES;

	if (Path[0] != L'\\') {
		Status = AppendFilePath(Path, FilePath);
		if (EFI_ERROR(Status))
			return Status;
	} else
		*FilePath = StrDup(Path);

	if (!*FilePath)
		return Status;

	return EFI_SUCCESS;
}
