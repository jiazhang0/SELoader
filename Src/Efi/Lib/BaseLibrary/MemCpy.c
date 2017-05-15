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
#include "BaseLibrary.h"

VOID *
MemCpy(VOID *Destination, CONST VOID *Source, UINTN MaxLength)
{
	UINTN UnalignedLen;
#ifdef __x86_64__
	register CHAR8 *d asm("rdi") = (CHAR8 *)Destination;
	register CHAR8 *s asm("rsi") = (CHAR8 *)Source;
#else
	register CHAR8 *d asm("edi") = (CHAR8 *)Destination;
	register CHAR8 *s asm("esi") = (CHAR8 *)Source;
#endif

	if (Destination == Source || !MaxLength)
		return Destination;

	asm volatile("cld");

	/* To ensure there is at least one dword/qword before running the
	 * faster dword/qword copy, choose THRESHOLD_SHORT_COPY dwords/qwords
	 * as the threshold.
	 */
	if (MaxLength <= THRESHOLD_SHORT_COPY) {
		asm volatile(".align 2\n\t"
			     "rep movsb\n\t"
			     :: "c"(MaxLength), "S"(s), "D"(d)
			     :"memory");
		return Destination;
	}

	/* Handle unaligned case */
	UnalignedLen = UNALIGNED(s);
	if (UnalignedLen && (UnalignedLen == UNALIGNED(d))) {
		UnalignedLen = sizeof(long) - UnalignedLen;
		MaxLength -= UnalignedLen;

		asm volatile(".align 2\n\t"
			     "rep movsb\n\t"
			     :: "c"(UnalignedLen), "S"(s), "D"(d)
			     : "memory");
	}

	/* Handle dword/qword copy */
#ifdef __x86_64__
	asm volatile(".align 4\n\t"
		     "rep movsq\n\t"
#else
	asm volatile(".align 2\n\t"
		     "rep movsl\n\t"
#endif
		     :: "c"((unsigned long)MaxLength / sizeof(long)), "S"(s), "D"(d)
		     : "memory");

	/* Handle the remaining bytes */
	MaxLength &= sizeof(long) - 1;
	asm volatile(".align 2\n\t"
		     "rep movsb\n\t"
		     :: "c"(MaxLength), "S"(s), "D"(d)
		     : "memory");

	return Destination;
}
