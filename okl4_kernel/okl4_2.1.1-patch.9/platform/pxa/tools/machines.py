##############################################################################
# Copyright (c) 2006, National ICT Australia
##############################################################################
# Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
# All rights reserved.
# 
# 1. Redistribution and use of OKL4 (Software) in source and binary
# forms, with or without modification, are permitted provided that the
# following conditions are met:
# 
#     (a) Redistributions of source code must retain this clause 1
#         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
#         (Licence Terms) and the above copyright notice.
# 
#     (b) Redistributions in binary form must reproduce the above
#         copyright notice and the Licence Terms in the documentation and/or
#         other materials provided with the distribution.
# 
#     (c) Redistributions in any form must be accompanied by information on
#         how to obtain complete source code for:
#        (i) the Software; and
#        (ii) all accompanying software that uses (or is intended to
#        use) the Software whether directly or indirectly.  Such source
#        code must:
#        (iii) either be included in the distribution or be available
#        for no more than the cost of distribution plus a nominal fee;
#        and
#        (iv) be licensed by each relevant holder of copyright under
#        either the Licence Terms (with an appropriate copyright notice)
#        or the terms of a licence which is approved by the Open Source
#        Initative.  For an executable file, "complete source code"
#        means the source code for all modules it contains and includes
#        associated build and other files reasonably required to produce
#        the executable.
# 
# 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
# LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
# IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
# EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
# THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
# BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
# PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
# THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
# 
# 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# ARM Platforms

class pxa(xscale):
    platform = "pxa"
    platform_dir = "pxa"
    drivers = xscale.drivers + ["pxa"]

class gumstix(pxa):
    virtual = False
    timer_driver = "pxa250_timer"
    rtc_driver = "pxa250_rtc"
    uart = "FFUART"
    serial_driver = "uart_8250"
    subplatform = "pxa255"
    drivers = [timer_driver] + pxa.drivers

    device_core = "gumstix"
    memory_timer  = [(0x40a00000, 0x40a01000)]
    interrupt_timer  = [27, 28]
    memory_serial = [(0x40100000, 0x40101000)]
    interrupt_serial = [22]
    memory_rtc = [(0x40900000, 0x40901000)]
    interrupt_rtc = [31]
    v2_drivers = [  (timer_driver, "vtimer", memory_timer, interrupt_timer),
                    (serial_driver, "vserial", memory_serial, interrupt_serial),
                    (rtc_driver, "vrtc", memory_rtc, interrupt_rtc)
                 ]

    skyeye = "gumstix.skyeye"
    memory = pxa.memory.copy()
    memory['physical']         = [Region(0xa0000000L, 0xa3800000L)]
    memory['rom']              = [Region(0xa3800000L, 0xa4000000L)]
    memory['pcmcia_cf_slot_0'] = [Region(0x20000000L, 0x30000000L, "dedicated")]
    memory['pcmcia_cf_slot_1'] = [Region(0x30000000L, 0x40000000L, "dedicated")]
    # memory['iodevices']        = [Region(0x40000000L, 0x5C000000L, "dedicated")]
    memory['sdram']            = [Region(0x5c000000L, 0x5c040000L, "dedicated")]
    cpp_defines = pxa.cpp_defines + [("PLATFORM_PXA", 1), ("SERIAL_FFUART", 1)]
    run_methods = copy.copy(pxa.run_methods)
    run_methods['hardware'] = gumstix_boot

