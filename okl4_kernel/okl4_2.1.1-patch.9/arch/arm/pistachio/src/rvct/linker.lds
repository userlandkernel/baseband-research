/*********************************************************************
 *
 * Copyright (C) 2006,  National ICT Australia (NICTA)
 *
 * File path:     platform/vesatile/rvct/linker.scl
 * Description:
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
#include <l4.h>
#include <tcb_layout.h>
#include <asmsyms.h>

LR_1 VIRT_ADDR_ROM NOCOMPRESS
{
    ER_START_ROM VIRT_ADDR_ROM
    {
	* (._start_rom)
    }

    ER_ROOT +0
    {
	* (.init.head)
    }

    ER_RO +0
    {
	.ANY (+RO)
    }

    ER_END_ROM +0
    {
	* (._end_rom)
    }

    ER_INIT +0
    {
	* (._start_init, +FIRST)
	* (.init)
	* (._end_init, +LAST)
    }

    .roinit +0
    {
	* (.roinit)
    }

#if VIRT_ADDR_ROM != VIRT_ADDR_RAM
}

LR_2 VIRT_ADDR_RAM NOCOMPRESS
{
    ER_START_RAM VIRT_ADDR_RAM
#else
    ER_START_RAM +0
#endif
    {
	* (._start_ram)
    }

    ER_GOT +0
    {
        * (._start_got, +FIRST)
        * (._end_got, +LAST)
    }

    ER_TRAPS +0 0x1000
    {
    * (.traps.data)
    }

    ER_RW +0
    {
	.ANY (+RW)
    }

    ER_KSPACE +0
    {
	* (.data.kspace)
    }

    ER_ZI +0
    {
	* (._start_bss, +FIRST)
	.ANY (+ZI)
	* (._end_bss, +LAST)
    }


    ER_END_RAM +0
    {
	* (._end_ram, +FIRST)
	* (._end, +LAST)
    }
}
