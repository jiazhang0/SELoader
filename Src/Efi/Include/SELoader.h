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

#ifndef SELOADER_H
#define SELOADER_H

#include <Efi.h>

#pragma pack(1)

#define SelSignatureRevision			1

typedef struct {
	UINT8 Revision;		/* signature format version */
	UINT32 HeaderSize;
	UINT32 TagDirectorySize;
	UINT32 NumberOfTag;
	UINT32 Flags;
} SEL_SIGNATURE_HEADER;

typedef struct {
	UINT32 Tag;
	UINT8 Revision;
	UINT8 Reserved[3];
	UINT32 DataOffset;
	UINT32 DataSize;
} SEL_SIGNATURE_TAG;

#define SelSignatureTagHashAlgorithm		1

typedef enum {
	SelHashAlgorithmSha1,
	SelHashAlgorithmSha224,
	SelHashAlgorithmSha256,
	SelHashAlgorithmSha384,
	SelHashAlgorithmSha512,
} SEL_SIGNATURE_HASH_ALGORITHM;

typedef struct {
	UINT8 Revision;
	SEL_SIGNATURE_HASH_ALGORITHM Algorithm;
} SEL_SIGNATURE_TAG_HASH_ALGORITHM;

#define SelSignatureTagSignatureAlgorithm	2

typedef enum {
	SelSignatureAlgorithmPkcs7,
} SEL_SIGNATURE_SIGNATURE_ALGORITHM;

typedef struct {
	UINT8 Revision;
	SEL_SIGNATURE_SIGNATURE_ALGORITHM Algorithm;
} SEL_SIGNATURE_TAG_SIGNATURE_ALGORITHM;

#define SelSignatureTagSignature		3

typedef struct {
	UINT8 Revision;
	UINT8 Signature[0];
} SEL_SIGNATURE_TAG_SIGNATURE;

#define SelSignatureTagSignaturePkcs7FlagsDetached	(1 << 0)

typedef struct {
	UINT8 Revision;
	UINT32 Flags;
	UINT8 Signature[0];
} SEL_SIGNATURE_TAG_SIGNATURE_PKCS7;

#define SelSignatureTagCreationTime		10
#define SelSignatureTagFileName			11
#define SelSignatureTagFileSize			12

#pragma pack()

#endif	/* SELOADER_H */
