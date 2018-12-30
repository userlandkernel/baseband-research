/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     pistachio-cvs/kernel/kdb/arch/ia32/tracebuffer.cc
 * Description:   Tracebuffer for IA-32
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

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <kdb/tracepoints.h>

#ifdef CONFIG_TRACEBUFFER

#ifdef CONFIG_PERFMON
#define IF_PERFMON(a...) a
#else
#define IF_PERFMON(a...)
#endif

DECLARE_CMD_GROUP (tracebuf);

DECLARE_CMD (cmd_tracebuffer, root, 'y', "tracebuffer", "Tracebuffer");

CMD(cmd_tracebuffer, cg)
{
    return tracebuf.interact (cg, "tracebuffer");
}

DECLARE_CMD (cmd_tb_dump, tracebuf, 'd', "dump", "Dump entire buffer");

CMD(cmd_tb_dump, cg)
{
    u32_t count = tracebuffer->current / sizeof(trace_t);
    u32_t index, top = 0, bottom = count, chunk = (count<32) ? count : 32;
    tracestatus_t old = { 0,0,0 }, sum = { 0,0,0 };
    trace_t *current;

    if (tracebuffer->magic != TBUF_MAGIC)
    {
        printf("Bad tracebuffer signature at %x\n",(u32_t)(&tracebuffer->magic));
        return CMD_NOQUIT;
    }  

    char c = get_choice ("Dump tracebuffer", "All/Region/Top/Bottom", 'b');
    switch (c)
    {
        case 'a' : 
            break;
        case 'r' : 
            top = get_dec("From record", 0, NULL);
            bottom = get_dec("To record", bottom, NULL);
            break;
        case 't' : 
            bottom = get_dec("Record count", chunk, NULL);
            break;
        default  : 
            top = bottom - get_dec("Record count", chunk, NULL);
            break;
    } 
    
    if (bottom > count)
        bottom = count;
    if (top > bottom) 
        top = bottom;

    printf("\nRecord  Cycles    " IF_PERFMON("UserInst  KrnlInst  ") "Event\n");
    for (index = top; index<bottom; index++)
    {
        current = &(tracebuffer->trace[index]);
        if (!old.cycles) 
            old = current->status;

        printf("%6d%10d" IF_PERFMON("%10d%10d") "  ", index,
            (current->status.cycles - old.cycles) 
            IF_PERFMON(,
                (current->status.pmc0 - old.pmc0), 
                (current->status.pmc1 - old.pmc1)
            )
        );

        sum.cycles += (current->status.cycles - old.cycles);
        sum.pmc0 += (current->status.pmc0 - old.pmc0);
        sum.pmc1 += (current->status.pmc1 - old.pmc1);
        old = current->status;

        switch ((current->identifier>>28) & 0xF)
        {
            case 0xF:
            {
                printf((char*)current->identifier, current->data[0], current->data[1], current->data[2], current->data[3]);
                break;
            }

            default:
            {
                printf("[%c%c%c%c]", (char)((current->identifier>>24)&0xFF), 
                                     (char)((current->identifier>>16)&0xFF), 
                                     (char)((current->identifier>>8)&0xFF),
                                     (char)(current->identifier&0xFF));

		c = (current->identifier>>24)&0xFF;
		if ((c>='1') && (c<='4'))
		{
		    u32_t i=0;
		    printf(", par=(0x%x", current->data[i++]);
		    while ((--c)>='1')
			printf(",0x%x", current->data[i++]);
		    printf(")");
		}
		break;
            }
        }
        
        printf("\n");  
    }
    
    printf("--------------------------------" IF_PERFMON("--------------------") "\n");  
    printf("      %10d" IF_PERFMON("%10d%10d") "  %6d entries\n\n", 
        sum.cycles, 
        IF_PERFMON(sum.pmc0, sum.pmc1,) 
        bottom-top
    );
    
    return CMD_NOQUIT;
}

#endif /* CONFIG_TRACEBUFFER */
