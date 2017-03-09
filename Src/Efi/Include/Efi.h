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

#ifndef EFI_H
#define EFI_H

#include <Edk2/Base.h>
#include <Edk2/Uefi.h>

#include <Edk2/Guid/GlobalVariable.h>
#include <Edk2/Guid/ImageAuthentication.h>
#include <Edk2/Guid/FileInfo.h>
#include <Edk2/Protocol/SimpleTextIn.h>
#include <Edk2/Protocol/LoadedImage.h>
#include <Edk2/Protocol/SimpleFileSystem.h>
#include <Edk2/Protocol/ServiceBinding.h>
#include <Edk2/Protocol/Hash2.h>
#include <Edk2/Protocol/Hash.h>
#include <Edk2/Protocol/Pkcs7Verify.h>

#ifdef GNU_EFI
#include <GnuEfi.h>
#endif

#endif	/* EFI_H */
