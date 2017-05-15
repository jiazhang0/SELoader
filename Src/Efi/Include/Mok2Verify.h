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

#ifndef EFI_MOK2_VERIFY_H
#define EFI_MOK2_VERIFY_H

#include <Efi.h>

#define EFI_MOK2_VERIFY_PROTOCOL_GUID	\
{	\
	0x4eda73ad, 0x07aa, 0x4b7a,	\
	{ 0xa1, 0x91, 0xd4, 0xd4, 0x10, 0xfb, 0x8c, 0xb4 }	\
}

typedef struct _EFI_MOK2_VERIFY_PROTOCOL	EFI_MOK2_VERIFY_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_MOK2_VERIFY_SIGNATURE) (
  IN EFI_MOK2_VERIFY_PROTOCOL *This,
  IN VOID                     *Signature,
  IN UINTN                    SignatureSize,
  IN VOID                     *Data,
  IN UINTN                    DataSize
  );

typedef
EFI_STATUS
(EFIAPI *EFI_MOK2_VERIFY_FILE_BUFFER) (
  IN EFI_MOK2_VERIFY_PROTOCOL *This,
  IN OUT VOID                 **Data,
  IN OUT UINTN                *DataSize,
  IN CONST CHAR16             *Path
  );

typedef
EFI_STATUS
(EFIAPI *EFI_MOK2_VERIFY_FILE) (
  IN EFI_MOK2_VERIFY_PROTOCOL *This,
  IN CONST CHAR16             *Path
  );

struct _EFI_MOK2_VERIFY_PROTOCOL {
        UINT8 Revision;
        EFI_MOK2_VERIFY_SIGNATURE VerifySignature;
        EFI_MOK2_VERIFY_FILE_BUFFER VerifyFileBuffer;
        EFI_MOK2_VERIFY_FILE VerifyFile;
};

extern EFI_GUID gEfiMok2VerifyProtocolGuid;

#endif	/* EFI_MOK2_VERIFY_H */
