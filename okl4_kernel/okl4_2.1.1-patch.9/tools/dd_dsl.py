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

from types import *
import sys
import wae_tok
import os
import traceback
import copy
import getopt

"""
Documentation:
<this will appear in thesis at some stage>

This is a parser for a domain specific language. Specifically this language
allows for the straightforward declaration of the register interface to a
device. This should make the task ofwriting device drivers simpler of the
programmer, more portable and less error prone.

It is easier for the programmer becaus a lot of the code that would previously
have to be written can be expressed in a simple format, which is very similar
to the way registers are defined in manuals and specifications.

After defining the interface the code generated is C code. This could be
retargted to other programming languages if required. More importantly it
can be retargetted to different hardware architectures.

Finally bit masking and shifting, while reasonably straight forward, and being
off by one can cause ``interesting'' effects on the hardware. By using this
technique it should be possible to avoid some of these bugs. This has been
shown to be the case in similar projects such as Devil.

Given that there is already the Devil project, why did I bother writing
something similar. The main reason is that Devil produced hideous Linux, x86
specific output. This wouldn't be a problem if I had access to the source,
unfortunately the developers have only released a binary of their compiler,
and efforts to collabrate were unsuccessful. None-the-less the result presented
by the devil paper and my own experience suggested that this was an important
tool to have in my quest to write Mungi device drivers.

Having done this, why not at least re-use their grammar? The reason for this
is that their language seems quite complicated and I'm not very experienced
with yacc|bison. Secondly for string maniuplation code I really prefer a higher
level language like python.

Conclusion:
I have implemented a very simple  and straight forward parser.

"""


"""
Grammar:
<register>      := <name> <port-desc> ":"
<port-desc>     := <expr> "@" <expr>

<variable>      := <name> <bit-desc>
<bit-desc>      := "<" <expr> ( : <expr> )?  ">"

<compound_var>  := VAR <name> ":"
"""

class Register:
    def __init__(self, space):
        self.space = space
    
    def __str__(self):
        ret =  "%s %s @ %s:" % (self.name, self.size, hex(self.offset))
        for var in self.vars:
            ret += "\n"
            ret += str(var)
        return ret

def get_read(size, offset, pre, var):
    return "%s_read_%d(%s, %s)" % (pre, size, var, offset)

def get_write(size, offset, pre, var, val):
    return "%s_write_%s (%s, (%s), %s)" % (pre, size, var, offset, val)


def write_type_info(reg, f, endian):
    #f.write("union %s {\n" % reg.name.lower())
    #f.write("\tuint%s_t reg;\n" % reg.size)
    f.write("struct %s_s{\n" % reg.name.lower())
    if endian == "LITTLE_ENDIAN":
        pos = 0
        pad_cnt = 0
        for var in reg.vars:
            # Work out padding
            if var.start > pos:
                # Need to add padding..
                pad_size = var.start - pos
                f.write("\tvolatile uint%d_t padding%s : %s;\n" % (reg.size, pad_cnt, pad_size))
                pad_cnt += 1
                pos += pad_size
            if var.end is None:
                size = 1
            else:
                size = var.end - var.start + 1
            f.write("\tvolatile uint%d_t %s : %s;\n" % (reg.size, var.name.lower(), size))
            pos += size
    elif endian == "BIG_ENDIAN":
        pos = reg.size - 1
        pad_cnt = 0
        reg_copy = copy.copy(reg.vars)
        reg_copy.reverse()
        for var in reg_copy:
            # Work out padding
            if var.end is None:
                var.end = var.start
            if var.end < pos:
                # Need to add padding..
                pad_size = pos - var.end
                f.write("\tvolatile uint%d_t padding%s : %s;\n" % (reg.size, pad_cnt, pad_size))
                pad_cnt += 1
                pos -= pad_size
            size = var.end - var.start + 1
            f.write("\tvolatile uint%d_t %s : %s;\n" % (reg.size, var.name.lower(), size))
            pos -= size
    else:
        raise Exception, "Unknown endian"
    f.write("};\n\n")
    f.write("union %s{\n" % reg.name.lower())
    f.write("\tstruct %s_s r;\n" % reg.name.lower())
    f.write("\tuint%d_t data;\n" % reg.size)
    f.write("};\n\n")


def write_print_info(reg, f, printf, specific):
    f.write("static inline void _%s_%s_print ( struct %s *space) {\n" % (reg.space, reg.name.lower(), reg.space))
    f.write('\t%s("Debug %s - 0x%%"PRIx%s"\\n", space->%s.data);\n' % (printf, reg.name.lower(), reg.size, reg.name.lower()))

    for var in reg.vars:
        f.write('\tif (space->%s.r.%s != 0) {\n\t\t%s("\\t%s: %%x\\n", space->%s.r.%s);\n\t}\n' %
                (reg.name.lower(),
                 var.name.lower(),
                 printf,
                 var.name.lower(),
                 reg.name.lower(),
                 var.name.lower()))
    f.write("}\n\n")

    if specific:
        f.write("#define %s_%s_print() _%s_%s_print(&%s)\n\n" %
                (reg.space, reg.name.lower(),
                 reg.space, reg.name.lower(), specific))


def write_access_info(reg, f, pre, specific):
    if specific is None:
        specific = "self->%s" % reg.space
    if reg.read:
        if len(reg.offset) == 1:
            f.write("static inline void _%s_%s_read(struct %s *%s) {\n" % \
                    (reg.space, reg.name.lower(), reg.space, reg.space))
            f.write("\t%s->%s.data = %s;\n" % (reg.space, reg.name.lower(),
                                                    get_read(reg.size, reg.offset[0], pre,
                                                             "%s->bus" % reg.space)))
            f.write("}\n\n");
            if specific:
                f.write("#define %s_%s_read() _%s_%s_read(&%s)\n\n" %
                        (reg.space, reg.name.lower(),
                         reg.space, reg.name.lower(), specific))


            # Now all the specific ones
            for var in reg.vars:
                f.write("static inline uint%s_t  _%s_%s_get(struct %s *%s) {\n" % \
                        (reg.size, reg.space, var.name.lower(), reg.space, reg.space))
                f.write("\t_%s_%s_read(%s);\n" % (reg.space, reg.name.lower(), reg.space))
                f.write("\treturn %s->%s.r.%s;\n" % (reg.space, reg.name.lower(), var.name.lower()))
                f.write("}\n\n");

                if specific:
                    f.write("#define %s_%s_get() _%s_%s_get(&%s)\n\n" %
                            (reg.space, var.name.lower(), reg.space,
                             var.name.lower(), specific))

        else:
            raise "Fix this!"
            array_name = "offset_array_%s_read" % reg.name.lower()
            f.write("static uintptr_t %s[%d] = {\n" % (array_name, len(reg.offset)))
            for each in reg.offset:
                f.write("\t%d,\n" % each)
            f.write("};\n\n")
            f.write("static inline union %s read_%s(int idx, bus_space_t bus) {\n" % \
                    (reg.name.lower(), reg.name.lower()))
            f.write("\tunion %s foo;\n" % reg.name.lower())
            f.write("\tfoo.data = %s;\n" % get_read(reg.size, "%s[idx]" % array_name, pre, "bus"))
            f.write("\treturn foo;\n")
            f.write("}\n\n");
            if specific:
                f.write("#define READ_%s(idx) read_%s(idx, %s)\n\n" %
                        (reg.name.upper(), reg.name.lower(), specific))

    if reg.write:
        if len(reg.offset) == 1:
            f.write("static inline void _%s_%s_write(struct %s *%s) {\n" %
                    (reg.space, reg.name.lower(), reg.space, reg.space))
            f.write("\t%s;\n" % get_write(reg.size, reg.offset[0], pre, "%s->bus" % reg.space,
                                                 "%s->%s.data" % (reg.space, reg.name.lower())))
            f.write("}\n\n");

            if specific:
                f.write("#define %s_%s_write() _%s_%s_write(&%s)\n\n" %
                        (reg.space, reg.name.lower(),
                         reg.space, reg.name.lower(), specific))

            # Now all the specific ones
            for var in reg.vars:
                f.write("static inline void _%s_%s_set(struct %s *%s, uint%s_t value) {\n" % \
                        (reg.space, var.name.lower(), reg.space, reg.space, reg.size))
                #f.write("\t_%s_%s_read(%s);\n" % (reg.space, reg.name.lower(), reg.space))
                f.write("\t%s->%s.r.%s = value;\n" % (reg.space, reg.name.lower(), var.name.lower()))
                f.write("\t_%s_%s_write(%s);\n" % (reg.space, reg.name.lower(), reg.space))
                f.write("}\n\n");

            
                if specific:
                    f.write("#define %s_%s_set(foo) _%s_%s_set(&%s, foo)\n\n" %
                            (reg.space, var.name.lower(), reg.space,
                             var.name.lower(), specific))
        else:
            array_name = "offset_array_%s_write" % reg.name.lower()
            f.write("static uintptr_t %s[%d] = {\n" % (array_name, len(reg.offset)))
            for each in reg.offset:
                f.write("\t%d,\n" % each)
            f.write("};\n\n")
            f.write("static inline void write_%s(int idx, bus_space_t bus, union %s foo) {\n" %
                    (reg.name.lower(), reg.name.lower()))
            f.write("\treturn %s;\n" % get_write(reg.size, "%s[idx]" % array_name, pre, "bus"))
            f.write("}\n\n");
            if specific:
                f.write("#define WRITE_%s(idx, foo) write_%s(idx, %s, foo)\n\n" %
                        (reg.name.upper(), reg.name.lower(), specific))
            

class Variable:

    def __str__(self):
        if self.end is None:
            return "\t%s <%s>" % (self.name, self.start)
        else:
            return "\t%s <%s:%s>" % (self.name, self.start, self.end)            

class ParseError(Exception):
    pass

class Parser:
    def __init__(self, name):
        self.name = name

    def parse_int_expr(self, toks):
        """Parse a list of tokens that is an expression and return an int"""
        try:
            ret = eval(" ".join(toks), {}, {})
        except:
            traceback.print_exc(file=sys.stdout)
            raise ParseError("Error evaluating expression: %s" % " ".join(toks))
        if type(ret) != IntType: raise ParseError("Expected 'int', got: %s" % ret)
        return ret

    def parse_list_expr(self, toks, parse_func):
        """Parse a list. Returns of list of whatever parse_func() returns"""
        list = []
        if toks[0] == '(':
            if toks[-1] != ')':
                raise ParseError("Error evaulating expression: %s" % " ".join(toks))
            pos_s = 1
            next_comma = 0
            while next_comma != -1:
                try:
                    next_comma = pos_s + toks[pos_s:].index(',')
                except ValueError:
                    next_comma = -1
                list.append(parse_func(toks[pos_s:next_comma]))
                pos_s = next_comma + 1
        else:
            list.append(parse_func(toks))
        return list

    def parse_register(self, toks):
        """Parse a list of tokens that we assume is a register"""
        reg = Register(self.name)
        reg.name = toks[0]

        try:
            offset = toks.index('@')
        except ValueError:
            raise ParseError("Expected an '@'")

        try:
            options_offset = toks.index('{')
            end_of_pos = options_offset
        except ValueError:
            options_offset = 0
            end_of_pos = -1

	if len(toks[1:offset]) == 0:
            raise ParseError("Expected size of the register")
        reg.size = self.parse_int_expr(toks[1:offset])
        reg.offset = self.parse_list_expr(toks[offset+1:end_of_pos], self.parse_int_expr)

        reg.read = 1
        reg.write = 1
        
        if options_offset:
            try:
                end_offset = toks.index('}')
            except ValueError:
                raise ParseError("Expected an '}'")
            for option in toks[options_offset+1:end_offset]:
                if option.upper() == 'RO':
                    " Read only so don't generate write macros "
                    reg.write = 0
                elif option.upper() == 'WO':
                    " Write only so don't generate read macros "
                    reg.read = 0
                elif option.upper() == 'RW':
                    " Read/write is default so don't do anything "
                    pass
                else:
                    raise ParseError("Unkown option: %s\n" % option)
        reg.debug = (self.line_no, self.file_name, self.line)

        return reg

    def parse_variable(self, toks):
        """Parse a variable expression"""
        var = Variable()
        var.name = toks[0]
        if toks[1] != "<": raise ParseError("Expected '<'")

        # Try and find a ':'
        toks = toks[2:]

        span = 0
        try:
            offset = toks.index(':')
            span = 1
            var.start = self.parse_int_expr(toks[:offset])
            toks = toks[offset+1:]
        except ValueError:
            span = 0

        # Try and find the closing '>'
        if toks[-1] != '>': raise ParseError("Closing '>' missing")

        val = self.parse_int_expr(toks[:-1])

        if span:
            var.end = val
        else:
            var.start = val
            var.end = None

        return var

    def parse(self, f):
        """Given a file returns a list of registers"""

        cur_reg = None
        registers = []
        tokeniser = wae_tok.tokeniser("#@<>:{}(),")

        # The format is brain-dead line based. So we just read
        # line by line.

        self.line_no = 1
        self.file_name = ""
        try:
            for self.line in f.xreadlines():
                # Ignore comments.
                if self.line.startswith('#'):
                    continue
		if self.line.startswith(';'):
                    # This is a meta command
                    (var, value) = self.line[1:-1].split("=")
                    globals()[var] = value
                    continue
                # Clear the tokeniser
                tokeniser.tokens = []
                # Get some more tokens
                tokeniser.tokenise(self.line)
                toks = tokeniser.tokens

                # Strip off comments at the end of the line
                if '#' in toks:
                    toks = toks[:toks.index("#")]

                # Parse 
                if len(toks) > 0 and toks[-1] == ":" and toks[0] == "VAR":
                    print "Found a variable!", toks
                    cur_reg = self.parse_compound(toks)
                elif len(toks) > 0 and toks[-1] == ":":
                    cur_reg = self.parse_register(toks)
                    cur_reg.vars = []
                    registers.append(cur_reg)
                elif len(toks) != 0:
                    assert cur_reg != None, "Missing register defn."
                    cur_reg.vars.append(self.parse_variable(toks))

                self.line_no += 1
                
        except ParseError, err_str:
            print "Parse Error:", err_str
            print "%s:%s %s" % (self.file_name, self.line_no, self.line)
            sys.exit(1)
        return registers

def usage():
        sys.stderr.write("Usage: dd_dsl.py [-o <output_dir>] <reg_file> "
                         "[<prefix>] [<handle>]")
        sys.exit(1)

def main(args):
    args
    opt_list, arg_list = getopt.getopt(args[1:], 'o:', ["api=", "arch=",
                                                        "space=", "handle=",
                                                        "endian="])
    if len(arg_list) != 1:
        usage()

    output_dir = None
    api = None
    space = "bus"
    handle = None
    arch = None
    endian = "LITTLE_ENDIAN"
    for opt, value in opt_list:
        if opt == '-o':
            output_dir = value
        elif opt == '--api':
            api = value
        elif opt == '--arch':
            arch = value
        elif opt == '--space':
            space = value
        elif opt == '--handle':
            handle = value
        elif opt == '--endian':
            endian = value

    name = arg_list[0]
    file_name = ".".join(name.split(".")[:-1])
    name = file_name.split("/")[-1]
    print "Name:", name
    parser = Parser(name)
    registers = parser.parse(open("%s.reg" % file_name))

    if globals().has_key('handle'): handle = globals()['handle']
    if globals().has_key('space'): space = globals()['space']    

    if (api == None or arch == None):
        pre = "%s" % space
    else:
        pre = "%s_%s_%s" % (api, arch, space)

    if (output_dir != None):
        output_file = os.path.join(output_dir, "%s_types.reg.h" % name)
    else:
        output_file = "%s_types.reg.h" % file_name

    types = open(output_file, "w")

    types.write("#ifndef _%s_TYPES_H_\n" % name.upper())
    types.write("#define _%s_TYPES_H_\n" % name.upper())

    for reg in registers:
        write_type_info(reg, types, endian)

    types.write("struct %s {\n\tbus_space_t bus;\n" % name)
    for reg in registers:
        types.write("\tunion %s %s;\n" % (reg.name.lower(), reg.name.lower()))
    types.write("};\n\n")

    types.write("#endif /* _%s_TYPES_H_ */\n" % name.upper())

    if (output_dir != None):
        output_file = os.path.join(output_dir, "%s.reg.h" % name)
    else:
        output_file = "%s.reg.h" % file_name

    header = open(output_file, "w")

    header.write("#ifndef _%s_H_\n" % name.upper())
    header.write("#define _%s_H_\n\n" % name.upper())
    header.write('#include "%s_types.reg.h"\n\n' % name.lower())

    header.write('#include <inttypes.h>\n\n')
    header.write('#include <driver/driver.h>\n\n')

    for reg in registers:
        write_access_info(reg, header, pre, handle)

    for reg in registers:
        write_print_info(reg, header, "printf", handle)

    header.write("#endif /* _%s_H_ */\n" % name.upper())

if __name__ == "__main__":
    main(sys.argv)
