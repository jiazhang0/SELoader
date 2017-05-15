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

#ifndef __LIB_INTERNAL_H__
#define __LIB_INTERNAL_H__

EFI_STATUS
UefiSecureBootDeployedMode(UINT8 *DeployedMode);

EFI_STATUS
UefiSecureBootAuditMode(UINT8 *AuditMode);

EFI_STATUS
UefiSecureBootSetupMode(UINT8 *SetupMode);

EFI_STATUS
UefiSecureBootState(UINT8 *SecureBoot);

EFI_STATUS
Pkcs7VerifyDetachedSignature(VOID *Hash, UINTN HashSize,
			     VOID *Signature, UINTN SignatureSize);

EFI_STATUS
Pkcs7VerifyAttachedSignature(VOID **SignedContent, UINTN *SignedContentSize,
			     VOID *Signature, UINTN SignatureSize);

EFI_STATUS
MokSecureBootState(UINT8 *MokSBState);

EFI_STATUS
MokVerifyProtocolInstalled(BOOLEAN *Installed);

EFI_STATUS
MokVerifyProtocolHook(VOID);

EFI_STATUS
Mok2VerifyInitialize(VOID);

EFI_STATUS
MokVerifyPeImage(VOID *Data, UINTN DataSize);

EFI_STATUS
SapHook(VOID);

EFI_STATUS
Sap2Hook(VOID);

VOID
SecurityPolicyInitialize(VOID);

#endif	/* __LIB_INTERNAL_H__ */
