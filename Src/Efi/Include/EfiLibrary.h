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

#ifndef EFI_LIBRARY_H
#define EFI_LIBRARY_H

#include <Efi.h>
#include <MokSecureBoot.h>
#include <SelSecureBoot.h>

extern EFI_HANDLE gThisImage;
extern EFI_HANDLE gThisDevice;
extern CHAR16 *gRootPath;

EFI_STATUS
EfiLibraryInitialize(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

VOID
EfiLibraryHexDump(CONST CHAR16 *Prompt, UINT8 *Data, UINTN DataSize);

EFI_STATUS
EfiMemoryAllocate(IN UINTN Size, OUT VOID **AllocatedBuffer);

VOID
EfiMemoryFree(VOID *Buffer);

EFI_STATUS
EfiProtocolOpen(EFI_HANDLE Handle, CONST EFI_GUID *Protocol, VOID **Interface);

EFI_STATUS
EfiProtocolLocate(CONST EFI_GUID *Protocol, VOID **Interface);

EFI_STATUS
EfiProtocolLocateHandles(CONST EFI_GUID *Protocol, EFI_HANDLE **HandleBuffer,
			 UINTN *Handles);

EFI_STATUS
EfiDeviceLocate(EFI_HANDLE *DeviceHandle);

EFI_STATUS
EfiVariableRead(CONST CHAR16 *VariableName, CONST EFI_GUID *VendorGuid,
		UINT32 *Attributes, VOID **Data, UINTN *DataSize);

STATIC inline EFI_STATUS
EfiVariableReadGlobal(CONST CHAR16 *VariableName, UINT32 *Attributes,
		      VOID **Data, UINTN *DataSize)
{
	return EfiVariableRead(VariableName, &gEfiGlobalVariableGuid,
			       Attributes, Data, DataSize);
}

STATIC inline EFI_STATUS
EfiVariableReadSecure(CONST CHAR16 *VariableName, UINT32 *Attributes,
		      VOID **Data, UINTN *DataSize)
{
	return EfiVariableRead(VariableName, &gEfiImageSecurityDatabaseGuid,
			       Attributes, Data, DataSize);
}

STATIC inline EFI_STATUS
EfiVariableReadMok(CONST CHAR16 *VariableName, UINT32 *Attributes,
		   VOID **Data, UINTN *DataSize)
{
	return EfiVariableRead(VariableName, &gMokSecureBootProtocolGuid,
			       Attributes, Data, DataSize);
}

STATIC inline EFI_STATUS
EfiVariableReadSel(CONST CHAR16 *VariableName, UINT32 *Attributes,
		   VOID **Data, UINTN *DataSize)
{
	return EfiVariableRead(VariableName, &gSelSecureBootProtocolGuid,
			       Attributes, Data, DataSize);
}

EFI_STATUS
EfiVariableWrite(CONST CHAR16 *VariableName, CONST EFI_GUID *VendorGuid,
		 UINT32 Attributes, VOID *Data, UINTN DataSize);

STATIC inline EFI_STATUS
EfiVariableWriteGlobal(CONST CHAR16 *VariableName, UINT32 Attributes,
		       VOID *Data, UINTN DataSize)
{
	return EfiVariableWrite(VariableName, &gEfiGlobalVariableGuid,
				Attributes, Data, DataSize);
}

STATIC inline EFI_STATUS
EfiVariableWriteSecure(CONST CHAR16 *VariableName, UINT32 Attributes,
		       VOID *Data, UINTN DataSize)
{
	return EfiVariableWrite(VariableName, &gEfiImageSecurityDatabaseGuid,
				Attributes, Data, DataSize);
}

STATIC inline EFI_STATUS
EfiVariableWriteMok(CONST CHAR16 *VariableName, UINT32 Attributes,
		    VOID *Data, UINTN DataSize)
{
	return EfiVariableWrite(VariableName, &gMokSecureBootProtocolGuid,
				Attributes, Data, DataSize);
}

STATIC inline EFI_STATUS
EfiVariableWriteSel(CONST CHAR16 *VariableName, UINT32 Attributes,
		    VOID *Data, UINTN DataSize)
{
	return EfiVariableWrite(VariableName, &gSelSecureBootProtocolGuid,
				Attributes, Data, DataSize);
}

EFI_STATUS
EfiVariableDelete(CONST CHAR16 *VariableName, CONST EFI_GUID *VendorGuid);

STATIC inline EFI_STATUS
EfiVariableDeleteGlobal(CONST CHAR16 *VariableName)
{
	return EfiVariableDelete(VariableName, &gEfiGlobalVariableGuid);
}

STATIC inline EFI_STATUS
EfiVariableDeleteSecure(CONST CHAR16 *VariableName)
{
	return EfiVariableDelete(VariableName, &gEfiImageSecurityDatabaseGuid);
}

STATIC inline EFI_STATUS
EfiVariableDeleteMok(CONST CHAR16 *VariableName)
{
	return EfiVariableDelete(VariableName, &gMokSecureBootProtocolGuid);
}

STATIC inline EFI_STATUS
EfiVariableDeleteSel(CONST CHAR16 *VariableName)
{
	return EfiVariableDelete(VariableName, &gSelSecureBootProtocolGuid);
}

EFI_STATUS
EfiDevicePathCreate(CONST CHAR16 *Path, CHAR16 **FilePath);

EFI_STATUS
EfiDevicePathRootDirectory(CHAR16 **DirectoryPath);

typedef enum {
	EFI_CPL_DEBUG,
	EFI_CPL_INFO,
	EFI_CPL_WARNING,
	EFI_CPL_ERROR,
	EFI_CPL_FAULT,
	EFI_CPL_MAX
} EfiConsolePrintLevel;

EFI_STATUS
EfiConsoleSetVerbosity(EfiConsolePrintLevel Level);

EFI_STATUS
EfiConsoleGetVerbosity(EfiConsolePrintLevel *Level);

UINTN
EfiConsolePrint(EfiConsolePrintLevel Level, CHAR16 *Format, ...);

#define EfiConsolePrintDebug(Format, ...)	\
	do {	\
		EfiConsolePrint(EFI_CPL_DEBUG, Format, ##__VA_ARGS__);	\
	} while (0)

#define EfiConsolePrintInfo(Format, ...)	\
	do {	\
		EfiConsolePrint(EFI_CPL_INFO, Format, ##__VA_ARGS__);  \
	} while (0)

#define EfiConsolePrintWarning(Format, ...)	\
	do {	\
		EfiConsolePrint(EFI_CPL_WARNING, Format, ##__VA_ARGS__);	\
	} while (0)

#define EfiConsolePrintError(Format, ...)	\
	do {	\
		EfiConsolePrint(EFI_CPL_ERROR, Format, ##__VA_ARGS__);	\
	} while (0)

#define EfiConsolePrintFault(Format, ...)	\
	do {	\
		EfiConsolePrint(EFI_CPL_FAULT, Format, ##__VA_ARGS__);	\
	} while (0)

UINTN
EfiConsoleVSPrint(CHAR16 *OutputBuffer, UINTN OutputBufferLen,
		  CONST CHAR16 *Format, VA_LIST Marker);

EFI_STATUS
EfiFileLoad(CONST CHAR16 *Path, VOID **Data, UINTN *DataSize);

EFI_STATUS
EfiFileSave(CONST CHAR16 *Path, VOID *Data, UINTN DataSize);

EFI_STATUS
EfiImageExecute(CONST CHAR16 *Path);

EFI_STATUS
EfiImageExecuteSecure(CONST CHAR16 *Path);

EFI_STATUS
EfiImageLoadDriver(CONST CHAR16 *Path);

VOID
EfiResetSystemWarm(VOID);

VOID
EfiStallMicroseconds(UINTN Microseconds);

VOID
EfiStallMilliseconds(UINTN Milliseconds);

VOID
EfiStallSeconds(UINTN Seconds);

EFI_STATUS
EfiSecurityPolicyLoad(CONST CHAR16 *Name, EFI_SIGNATURE_LIST **SignatureList,
		      UINTN *SignatureListSize);

EFI_STATUS
EfiSecurityPolicyFree(EFI_SIGNATURE_LIST **SignatureList);

VOID
EfiSecurityPolicyPrint(VOID);

EFI_STATUS
SelSecureBootMode(UINT8 *SelSecureBoot);

EFI_STATUS
EfiSignatureVerifyBuffer(VOID *Signature, UINTN SignatureSize,
			 VOID *Data, UINTN DataSize);

EFI_STATUS
EfiSignatureVerifyAttached(VOID *Signature, UINTN SignatureSize,
			   VOID **Data, UINTN *DataSize);

typedef struct {
	EFI_GUID *HashAlgorithm;
	VOID *Hash;
	UINTN HashSize;
	BOOLEAN Extended;
	UINTN HashedDataSize;
} EFI_HASH_CONTEXT;

EFI_STATUS
EfiHashSize(CONST EFI_GUID *HashAlgorithm, UINTN *HashSize);

EFI_STATUS
EfiHashInitialize(CONST EFI_GUID *HashAlgorithm,
		  EFI_HASH_CONTEXT *Context);

EFI_STATUS
EfiHashUpdate(EFI_HASH_CONTEXT *Context, CONST UINT8 *Message,
	      UINTN MessageSize);

EFI_STATUS
EfiHashFinalize(EFI_HASH_CONTEXT *Context, UINT8 **Hash, UINTN *HashSize);

EFI_STATUS
EfiHashData(CONST EFI_GUID *HashAlgorithm, CONST UINT8 *Message,
	    UINTN MessageSize, UINT8 **Hash, UINTN *HashSize);

#endif	/* EFI_LIBRARY_H */
