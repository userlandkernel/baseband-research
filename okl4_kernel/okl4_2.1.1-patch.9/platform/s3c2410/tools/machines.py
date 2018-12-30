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

class s3c2410(arm920t):
    platform = "s3c2410"
    platform_dir = "s3c2410"
    memory = arm920t.memory.copy()
    timer_driver = "s3c2410_timer"
    rtc_driver = "s3c2410_rtc"
    serial_driver = "s3c2410_uart"
    lcd_driver = "s3c2410_lcd"
    touch_driver = "s3c2410_touch"
    spi_driver = "s3c2410_spi"
    drivers = [timer_driver, serial_driver, lcd_driver]
    memory_timer = [(0x51000000, 0x51001000)]
    interrupt_timer = [11, 12]
    memory_serial = [(0x50000000, 0x50001000)]
    interrupt_serial = [32, 33]
    memory_lcd = [(0x4d000000, 0x4d001000)]
    interrupt_lcd = [16]
    memory_rtc = [(0x57000000, 0x57001000)]
    interrupt_rtc = [30]
    v2_drivers = [("s3c2410_timer", "vtimer", memory_timer, interrupt_timer),
                 ("s3c2410_uart", "vserial", memory_serial, interrupt_serial),
                 ("s3c2410_lcd", "vlcd", memory_lcd, interrupt_lcd),
                 ("s3c2410_rtc", "vrtc", memory_rtc, interrupt_rtc)
                 # ("s3c2410_spi", "vbus"),
                 # ("s3c2410_touch", "vtouch"),
                 ]


class gta01(s3c2410):
    virtual = False
    subplatform = "gta01"

    skyeye = "gta01.skyeye"
    device_core = "gta01"
    memory = s3c2410.memory.copy()
    memory['physical'] = [Region(0x30000000L, 0x38000000L)]
#    memory['reserved'] = [Region(0x60000000L, 0xffffffffL, "reserved")]
#    memory['sfr']      = [Region(0x48000000L, 0x60000000L, "dedicated")]
    memory['rom1']      = [Region(0x08000000L, 0x30000000L, "dedicated")]
    memory['sdram']    = [Region(0x00000000L, 0x00001000L, "dedicated")]
    cpp_defines = s3c2410.cpp_defines + [("PLATFORM_S3C2410", 1), ("SERIAL_UART", 0), ("CONFIG_C15_CLOCK","C15_CONTROL_CK_ASync")]

    run_methods = {
        "qemu" : qemu_gta_sim,
        "skyeye" : skyeye_sim
    }
    default_method = "qemu"

# This is the same as the original gta01, except we name some of the RAM as 'ROM' and
# use this to run the rommable regressions
class gta01_xip(gta01):
    # we choose to have 2MB of fake ROM
    memory = s3c2410.memory.copy()
    memory['rom'] = [Region(0x30000000L, 0x30200000L)]
    memory['physical'] = [Region(0x30200000L, 0x38000000L)]


