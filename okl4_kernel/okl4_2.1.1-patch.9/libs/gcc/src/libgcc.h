/*********************************************************************
 *
 * Copyright (C) 2007,  Open Kernel Labs Inc.
 *
 * Description:  Libgcc Declarations
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 ********************************************************************/
uint64_t __udivdi3(uint64_t num, uint64_t den);
uint32_t __udivsi3(uint32_t num, uint32_t den);
uint64_t __umoddi3(uint64_t num, uint64_t den);
uint32_t __umodsi3(uint32_t num, uint32_t den);
int64_t  __divdi3(int64_t num, int64_t den);
int32_t  __divsi3(int32_t num, int32_t den);
int64_t  __moddi3(int64_t num, int64_t den);
int32_t  __modsi3(int32_t num, int32_t den);
uint64_t __ashldi3(uint64_t v, int cnt);
uint64_t __ashrdi3(uint64_t v, int cnt);
uint64_t __lshrdi3(uint64_t v, int cnt);
uint32_t __clzdi2(uint64_t v);
uint32_t __clzsi2(uint32_t v);
uint64_t __fixunsdfdi(double x);
uint64_t __fixunssfdi(float f);

void __divide_error(void);
