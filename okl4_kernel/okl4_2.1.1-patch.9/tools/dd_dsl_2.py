#!/usr/bin/env python
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
###############################################################################


###############################################################################
#
# Generate code to access the memory-mapped registers of a hardware device
# based on an XML definition (normally in a "*.dx" file).
#
# The documentation at the head of dd_dsl.py give more detail, though the
# syntax is somewhat changed now.
#
###############################################################################

import sys, sets
from ezxml import Element, long_attr, str_attr

def uniq(lst):
    return list(sets.Set(lst))

###############################################################################
# Define the XML elements
###############################################################################
resource_el = Element("resource",
                      name=(str_attr, "required"),
                      rtype=(str_attr, "required"))

state_el = Element("state",
                   name=(str_attr, "required"))

field_el = Element("field",
                 name=(str_attr, "required"),
                 bits=(str_attr, "required"),
                 )

reg_el = Element("register",
                 field_el,
                 name=(str_attr, "required"),
                 offset=(long_attr, "required"),
                 type=(str_attr, "required")
                 )

f_reg_el = Element("fake_register",
                 field_el,
                 name=(str_attr, "required"),
                 )

block_el = Element("block", reg_el, f_reg_el,
                   name=(str_attr, "optional"),
                   space_type=(str_attr, "required"),
                   space=(str_attr, "required"),
                   register_size=(long_attr, "required"))

datafield_el = Element("datafield",
                       ftype=(str_attr, "required"),
                       name=(str_attr, "required"))

interface_el = Element("interface",
                       interface_type=(str_attr, "required"),
                       name=(str_attr, "required"))

device_el = Element("device",
                    block_el, interface_el, datafield_el,
                    state_el, resource_el,
                    name=(str_attr, "required"))

###############################################################################
# Templates
###############################################################################
headers = """
#include <assert.h>
#include <driver/driver.h>
#include <stdio.h>
#include <inttypes.h>
#include <util/format_bin.h>

"""

struct_driver = """
struct driver %(name)s = {
   \"%(name)s\",
   sizeof(struct %(name)s),
   &device_ops
};

"""

register_read = """
static inline uint%(size)d_t
%(regname)s_read_(%(spacetype)s_space_t space) {
    uint%(size)d_t value;
    value =  %(spacetype)s_space_read_%(size)d(space, 0x%(offset)lx);
#if defined(DEBUG_DDL)
    printf(\"\\t\\tRead %%x from register %(regname)s\\n\", value);
#endif
    return value;
}

#define %(regname)s_read() %(regname)s_read_(%(defaultspace)s.resource.%(spacetype)s)

"""

register_get_bit = """
static inline int
%(regname)s_get_bit_(%(spacetype)s_space_t space, uint8_t bit) {
    int tmp = %(regname)s_read_(space) & (1 << bit);
#if defined(DEBUG_DDL)
    printf(\"Getting bit %(regname)s:%%d -- %%d\\\n\", bit, tmp);
#endif
    return tmp;
}

#define %(regname)s_get_bit(value) %(regname)s_get_bit_(%(defaultspace)s.resource.%(spacetype)s, value)
"""
set_offline_register = """
static inline void
%s_set_var(uint32_t val) {
    %s = val;
}
"""

register_offline_get_bit = """
static inline int
%(regname)s_get_bit_(uint32_t val, uint8_t bit) {
    int tmp = val & (1 << bit);
#if defined(DEBUG_DDL)
    printf(\"Getting bit %(regname)s:%%d -- %%d\\\n\", bit, tmp);
#endif
    return tmp;
}

#define %(regname)s_get_bit(value) %(regname)s_get_bit_(%(defaultvar)s.resource.%(spacetype)s, value)
"""

register_write = """
static inline void
%(regname)s_write_(%(spacetype)s_space_t space, uint%(size)d_t value) {
#if defined(DEBUG_DDL)
    printf(\"\\tWriting %%x to register %(regname)s\\n\", value);
#endif
    %(spacetype)s_space_write_%(size)d(space, 0x%(offset)lx, value);
}

#define %(regname)s_write(value) %(regname)s_write_(%(defaultspace)s.resource.%(spacetype)s, value)

"""

register_set_bit = """
static inline void
%(regname)s_set_bit_(%(spacetype)s_space_t space, uint8_t bit) {
#if defined(DEBUG_DDL)
    printf(\"Setting bit %(regname)s:%%d\\\n\", bit);
#endif
    uint%(size)d_t tmp = %(regname)s_read_(space);
    tmp |= (1 << bit);
    %(regname)s_write_(space, tmp);
}

#define %(regname)s_set_bit(value) %(regname)s_set_bit_(%(defaultspace)s.resource.%(spacetype)s, value)

"""

register_clear_bit = """
static inline void
%(regname)s_clear_bit_(%(spacetype)s_space_t space, uint8_t bit) {
#if defined(DEBUG_DDL)
    printf(\"Clear bit %(regname)s:%%d\\\n\", bit);
#endif
    uint%(size)d_t tmp = %(regname)s_read_(space);
    tmp &= ~(1 << bit);
    %(regname)s_write_(space, tmp);
}

#define %(regname)s_clear_bit(value) %(regname)s_clear_bit_(%(defaultspace)s.resource.%(spacetype)s, value)

"""

field_get = """
static inline uint%(size)d_t
%(regname)s_get_%(fieldname)s_(%(spacetype)s_space_t space) {
    uint%(size)d_t tmp = %(regname)s_read_(space);
    tmp = (tmp >> %(shift)d) & 0x%(mask)x;
#if defined(DEBUG_DDL)
    printf(\"\\tGetting field %(regname)s:%(fieldname)s -- %%x\\n\", tmp);
#endif
    return tmp;
}

#define %(regname)s_get_%(fieldname)s() %(regname)s_get_%(fieldname)s_(%(defaultspace)s.resource.%(spacetype)s)

"""

field_offline_get = """
static inline uint%(size)d_t
%(regname)s_get_%(fieldname)s_(uint%(size)d_t var) {
    uint%(size)d_t tmp = (var >> %(shift)d) & 0x%(mask)x;
#if defined(DEBUG_DDL)
    printf(\"\\tGetting field %(regname)s:%(fieldname)s -- %%x\\n\", tmp);
#endif
    return tmp;
}

#define %(regname)s_get_%(fieldname)s() %(regname)s_get_%(fieldname)s_(%(defaultvar)s)
"""

field_set = """
static inline void
%(regname)s_set_%(fieldname)s_(%(spacetype)s_space_t space, uint%(size)d_t value) {
#if defined(DEBUG_DDL)
    printf(\"\\tSetting field %(regname)s:%(fieldname)s to %%x\\n\", value);
#endif
    uint%(size)d_t tmp = %(regname)s_read_(space);
    tmp &= ~ (0x%(mask)x << %(shift)d);
    tmp |= ((value & 0x%(mask)x) << %(shift)d);
    %(regname)s_write_(space, tmp);
}

#define %(regname)s_set_%(fieldname)s(value) %(regname)s_set_%(fieldname)s_(%(defaultspace)s.resource.%(spacetype)s, value)
"""

################################################################################
# The actual parsing stuff
################################################################################
class RegFile:
    def __init__(self, file, output_file, arch):
        device = device_el.parse_xml_file(file)

        output = open(output_file, "w")
        
        name = device.name
        interfaces = uniq([interface.interface_type for interface in
                           device.find_children('interface')]) + ["device"]

        # Print out header files
        output.write(headers)
        output.write("#include <driver/mem_space.h>\n")
        for iface in interfaces:
            output.write("#include <driver/%s.h>\n" % iface)

        output.write("enum %s_state {\n" % name)
        for state in device.find_children("state"):
            output.write("  STATE_%s,\n" % state.name.upper())
        output.write("};\n")
        output.write("struct %s {\n" % name)
        output.write("    struct device_interface device;\n")
        for iface in device.find_children('interface'):
            output.write("    struct %s_interface %s;\n" % \
                         (iface.interface_type, iface.name))
        output.write("    enum %s_state state;\n" % name);
        for resource in device.find_children('resource'):
            if resource.rtype == "x86_ioport_space_t" and not (arch == "ia32" or arch == "x86_64"):
                print "Can't use x86_ioport_space_t on", arch
                sys.exit(1)
            output.write("    struct resource %s;\n" % (resource.name))
        for field in device.find_children("datafield"):
            output.write("    %s %s;\n" % (field.ftype, field.name))
        output.write("};\n\n")

        for iface in interfaces:
            output.write("SETUP_%s_PROTOS(%s)\n" % (iface.upper(), name))
            output.write("SETUP_%s_OPS()\n" % iface.upper())

        # Print out the driver structure
        output.write(struct_driver % {"name" : name })


        blocks =  device.find_children('block')
        for block in blocks:
            regs = block.find_children("register")
            default_space = "device->%s" % block.space
            for reg in regs:
                self.output_reg(output, reg, block.space_type, default_space, block.register_size)
            if hasattr(block, "name"):
                print >>output, "static inline void"
                print >>output, "%s_print_(%s_space_t space) {" % (block.name, block.space_type)
                print >>output, '    printf("[Block %s]\\n");' % block.name
                for reg in regs:
                    if reg.type in ["rw", "ro"]:
                        print >>output, "    %s_print_(space);" % reg.name
                print >>output, '}'
                print >>output
                print >>output, "#define %s_print() %s_print_(%s.resource.%s)" % \
                    (block.name, block.name, default_space, block.space_type)

            regs = block.find_children("fake_register")
            for reg in regs:
                default_var = "var_%s" % reg.name
                print >> output, "\nstatic uint32_t %s;\n" % default_var

                print >> output, set_offline_register % (reg.name, default_var)
                reg.type = "sro"
                reg.offset = 0x0
                self.output_reg(output,
                                reg,
                                block.space_type,
                                "var_%s" % reg.name,
                                block.register_size)

    def output_reg(self, output, reg, space_type, default_space, default_size):
        def get_size_shift_mask(field):
            field_size = 0
            bits = field.bits
            if ':' in bits:
                minb, maxb = bits.split(":")
                minb = int(minb)
                maxb = int(maxb)
            else:
                minb = maxb = int(bits)
            shift = minb
            mask_bits = maxb - minb + 1
            mask = (2 ** mask_bits) - 1
            field_size = maxb - minb + 1
            return field_size, shift, mask
            
        size = default_size;

        for field in reg.find_children("field"):
            field_size, shift, mask = get_size_shift_mask(field)
            # <regname>_set_<fieldname>
            if field_size == 1:
                print >>output, "#define %s_%s (uint%d_t) (1 << %d)" % \
                      (reg.name.upper(), field.name.upper(), size, shift)
            else:
                print >>output, "#define %s_%s(value) (uint%d_t) ((value & 0x%x) << %d)" % \
                      (reg.name.upper(), field.name.upper(), size, mask, shift)

        reg_dict = { "regname":reg.name,
                     "spacetype":space_type,
                     "size":size,
                     "offset":reg.offset,
                     "defaultspace":default_space }
        
        if reg.type in ["rw", "ro"]:
            # output <regname>_read()
            output.write(register_read % reg_dict)

            # output <regname>_get_bit(bit)
            output.write(register_get_bit % reg_dict)

            for field in reg.find_children("field"):
                field_size, shift, mask = get_size_shift_mask(field)
                # <regname>_get_<fieldname>
                field_dict = dict(reg_dict)
                field_dict["fieldname"] = field.name
                field_dict["shift"] = shift
                field_dict["mask"] = mask                
                output.write(field_get % field_dict)

            #output.write(regname_print_start % reg_dict)
            print >>output, "static inline void"
            print >>output, "%s_print_(%s_space_t space) {" % (reg.name, space_type)
            print >>output, '    uint%d_t tmp = %s_read_(space);' % (size, reg.name)
            print >>output, '    printf("%-23s: 0x%%0%d" PRIx%d " %%s\\n", tmp, fbin_%s(tmp));' % \
                  (reg.name, size / 4, size, size)
            for field in reg.find_children("field"):
                name = field.name
                field_size, shift, mask = get_size_shift_mask(field)
                print >>output, '    printf("\t%-15s: %%" PRId%d "\\n", ((tmp >> %d) & 0x%x));' % (name, size, shift, mask)

            #output.write(regname_print_end % reg_dict)
            print >>output, '}'
            print >>output
            print >>output, "#define %s_print() %s_print_(%s)" % \
                  (reg.name, reg.name, default_space)

        if reg.type in ["sro"]:
            reg_dict["defaultvar"] = default_space
            output.write(register_offline_get_bit % reg_dict)
            for field in reg.find_children("field"):
                field_size, shift, mask = get_size_shift_mask(field)
                # <regname>_get_<fieldname>
                field_dict = dict(reg_dict)
                field_dict["fieldname"] = field.name
                field_dict["shift"] = shift
                field_dict["mask"] = mask                
                output.write(field_offline_get % field_dict)

            #output.write(regname_print_start % reg_dict)
            print >>output, "static inline void"
            print >>output, "%s_print_(uint%d_t var) {" % (reg.name, size)
            print >>output, '    uint%d_t tmp = var;' % size
            print >>output, '    printf("%-23s: 0x%%0%d" PRIx%d " %%s\\n", tmp, fbin_%s(tmp));' % \
                  (reg.name, size / 4, size, size)
            for field in reg.find_children("field"):
                name = field.name
                field_size, shift, mask = get_size_shift_mask(field)
                print >>output, '    printf("\t%-15s: %%" PRId%d "\\n", ((tmp >> %d) & 0x%x));' % (name, size, shift, mask)

            #output.write(regname_print_end % reg_dict)
            print >>output, '}'
            print >>output
            print >>output, "#define %s_print() %s_print_(%s)" % \
                  (reg.name, reg.name, default_space)
            

        if reg.type in ["rw", "wo"]:
            # <regname>_write
            output.write(register_write % reg_dict)

        if reg.type in ["rw"]:
            # <regname>_set_bit
            output.write(register_set_bit % reg_dict)

            # <regname_clear_bit>
            output.write(register_clear_bit % reg_dict)

            for field in reg.find_children("field"):
                field_size, shift, mask = get_size_shift_mask(field)
                # <regname>_set_<fieldname>
                field_dict = dict(reg_dict)
                field_dict["fieldname"] = field.name
                field_dict["shift"] = shift
                field_dict["mask"] = mask                
                output.write(field_set % field_dict)

r = RegFile(sys.argv[1], sys.argv[2], sys.argv[3])
