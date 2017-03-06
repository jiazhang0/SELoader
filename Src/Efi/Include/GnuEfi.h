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

#ifndef GNU_EFI_H
#define GNU_EFI_H

#include <Efi.h>

VOID
InitializeLib(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
);

UINTN
VPrint(
	IN CONST CHAR16 *fmt,
	VA_LIST args
);

INTN
StrCmp(
	IN CONST CHAR16 *s1,
	IN CONST CHAR16 *s2
);

UINTN
StrLen(
	IN CONST CHAR16 *s1
);

VOID
StrCpy(
	IN CHAR16 *Dest,
	IN CONST CHAR16 *Src
);

EFI_FILE_INFO *
LibFileInfo(
	IN EFI_FILE_HANDLE FHand
);

EFI_DEVICE_PATH *
FileDevicePath(
	IN EFI_HANDLE Device OPTIONAL,
	IN CHAR16 *FileName
);

CHAR16 *
DevicePathToStr(
	EFI_DEVICE_PATH *DevPath
);

#endif	/* GNU_EFI_H */
