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

#ifndef EFI_MOK_VERIFY_H
#define EFI_MOK_VERIFY_H

#include <Efi.h>

#define EFI_MOK_VERIFY_PROTOCOL_GUID	\
{	\
	0x605dab50, 0xe046, 0x4300,	\
	{ 0xab, 0xb6, 0x3d, 0xd8, 0x10, 0xdd, 0x8b, 0x23 }	\
}

typedef struct _EFI_MOK_VERIFY_PROTOCOL	EFI_MOK_VERIFY_PROTOCOL;

typedef
EFI_STATUS
(*EFI_MOK_VERIFY_IMAGE) (
  IN VOID *Buffer,
  IN UINT32 BufferSize
  );

typedef
EFI_STATUS
(*EFI_MOK_HASH) (
  IN UINT8                     *Data,
  IN UINTN                     DataSize,
  PE_COFF_LOADER_IMAGE_CONTEXT *Context,
  UINT8                        *Sha256Hash,
  UINT8                        *Sha1Hash
  );

typedef
EFI_STATUS
(*EFI_MOK_CONTEXT) (
  IN VOID                         *Data,
  IN UINTN                        DataSize,
  IN PE_COFF_LOADER_IMAGE_CONTEXT *Context
  );

struct _EFI_MOK_VERIFY_PROTOCOL {
        EFI_MOK_VERIFY_IMAGE Verify;
        EFI_MOK_HASH Hash;
        EFI_MOK_CONTEXT Context;
};

extern EFI_GUID gEfiMokVerifyProtocolGuid;

#endif	/* EFI_MOK_VERIFY_H */
