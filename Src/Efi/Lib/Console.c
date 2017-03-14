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

STATIC EfiConsolePrintLevel CurrentConsolePrintLevel = EFI_CPL_DEBUG;

STATIC UINTN
ConsolePrint(EfiConsolePrintLevel Level, CHAR16 *Format, VA_LIST Marker)
{
	if (Level < CurrentConsolePrintLevel)
		return 0;

	return VPrint(Format, Marker);
}

UINTN
EfiConsolePrint(EfiConsolePrintLevel Level, CHAR16 *Format, ...)
{
	VA_LIST Marker;

	VA_START(Marker, Format);
	UINTN OutputLength = ConsolePrint(Level, Format, Marker);
	VA_END(Marker);

	return OutputLength;
}

UINTN
EfiConsoleTrace(EfiConsolePrintLevel Level, CHAR16 *Format, ...)
{
	UINTN OutputLength = 0;

	if (Format) {
		VA_LIST Marker;

		VA_START(Marker, Format);
		OutputLength = ConsolePrint(Level, Format, Marker);
		VA_END(Marker);
	}

#ifdef TRACE_BUILD
	CHAR16 Typed;
	CHAR16 *Prompt;

	if (Format)
		Prompt = L" >>|\r\n";
	else
		Prompt = L">>|\r\n";

	Input(Prompt, &Typed, 1);
#else
	/* The trace prompt usually doesn't end with \n */
	OutputLength += EfiConsolePrint(Level, L"\n");
#endif
	return OutputLength;
}

EFI_STATUS
EfiConsoleSetVerbosity(EfiConsolePrintLevel Level)
{
	if (Level < EFI_CPL_DEBUG || Level >= EFI_CPL_MAX)
		return EFI_INVALID_PARAMETER;

	CurrentConsolePrintLevel = Level;

	return EFI_SUCCESS;
}

EFI_STATUS
EfiConsoleGetVerbosity(EfiConsolePrintLevel *Level)
{
	if (!Level)
		return EFI_INVALID_PARAMETER;

	if (CurrentConsolePrintLevel < EFI_CPL_DEBUG ||
	    CurrentConsolePrintLevel >= EFI_CPL_MAX)
		return EFI_INVALID_PARAMETER;

	*Level = CurrentConsolePrintLevel;

	return EFI_SUCCESS;
}
