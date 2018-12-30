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

"""
The Elfadorn script turns orphaned sections in object files into segments
in the output file by adding to the linker script.
"""

import sys
import os
import optparse
from elf.core import UnpreparedElfFile
from elf.section import UnpreparedElfStringTable
from elf.constants import PF_R, PF_W, PF_X, PT_LOAD

from adorn.arguments import get_script_names, remove_arguments, \
     get_linker_name
from adorn.sections import get_section_names, remove_sections_wildcard
from adorn.linkerscript import get_linker_script_text, \
     linker_script_sections,  linker_script_segments, write_linker_script
import parsers.rvct.rvct_parser as rvct_parser
import parsers.gnu.gnu_parser as gnu_parser

def main(args):
    """Main program entry point."""

    # We should be able ot use 'elfadorn ld' as a drop-in replacement for 'ld'
    # Here we detect if this is the case, and patch the command line args appropriately.
    # This way we avoid maintainid two different methods of dealing with args
    if "--" not in args:
        args = [args[0] , "--"] + args[1:]

    parser = optparse.OptionParser("%prog [options] -- <linker> [linker_options]",
                                   add_help_option=0)
    parser.add_option("-H", "--help", action="help")
    parser.add_option("-o", "--output", dest="target", metavar="FILE",
                        help="Linker will create FILE.")
    parser.add_option("-f", "--file-segment-list", dest="file_segment_list", metavar="FILE",
                        help="File containing segment names to be added to .segment_names, \
                        one per line")
    parser.add_option("-c", "--cmd-segment-list", dest="cmd_segment_list",
                        help="quoted list of comma separated segment names to be added to .segment_names,")

    parser.add_option("-s", "--create-segments", dest="create_segments", action="store_true",
                        help="Set to enable gathering orphaned sections and placing each in a new segment")

    (options, args) = parser.parse_args(args)



    if not options.target:
        i = 0
        for a in args:
            if a == "-o":
                options.target = args[i+1]
                break
            i = i + 1

        if not options.target:
            print "Error: -o flag must be supplied." 
            sys.exit(1)


    # we need to parse the options
    # we are interested in any -T, --scatter or --script= options
    # plus the ordinary files specified on the command line    
    scripts = get_script_names(args)

    objects = remove_arguments(args)
    linker_name = objects[1]
    objects = objects[2:]

    linker_type = get_linker_name(args, linker_name)
    if linker_type == "rvct":
        if options.create_segments:
            print "Warning: creating segments from sections not applicable to RVCT. Disabling option."
        options.create_segments = False

    # next get section names
    (sections, additional_scripts) = get_section_names(objects)
    scripts = scripts + additional_scripts

    # then get the text of the linker script
    script_text = get_linker_script_text(linker_name, scripts, additional_scripts, [])

    if options.create_segments:
        # get rid of any sections named in the script_text
        mentioned_sections = linker_script_sections(script_text)
        orphaned_sections = sections
        for section in mentioned_sections:
        # Our grammar is not perfect, sometimes it gets confused and gives back *
        # as a section but it is actually a filename
            if section != "*":
                remove_sections_wildcard(orphaned_sections, section)

        # mips-ld treats .reginfo sections somewhat magically, we do not want to treat this
        # as an orphan and create a segment for him, else ld will drop the text data and bss
        # sections completely. Magic.
        if '.reginfo' in orphaned_sections:
            orphaned_sections.remove('.reginfo')



        # work out the new linker command line
        if scripts == []:
            default = get_linker_script_text(args[1], [], [], args[2:])
            open("default.lds", "w").write(default)
            if len(orphaned_sections) != 0:
                args += ["--script=default.lds"]

        # write out an additional linker script file to pass to the linker
        if len(orphaned_sections) != 0:
            write_linker_script(orphaned_sections, "additional.lds")
            additional_scripts.append("additional.lds")
            args += ["--script=additional.lds"]
    else:       
        # if we dont care about these, just say there are none.
        orphaned_sections = []

    # execute the linker
    if os.spawnvp(os.P_WAIT, args[1], args[1:]) != 0:
        sys.exit(1)

    # load the elf file
    elf = UnpreparedElfFile(filename=options.target)
    wordsize = elf.wordsize
    endianess = elf.endianess
    seglist = get_segment_names(options, elf, scripts, linker_type, orphaned_sections)

    # create the string table
    segname_tab = UnpreparedElfStringTable(".segment_names")

    # add the segment names
    for segname in seglist:
        segname = segname.strip()
        segname_tab.add_string("%s" % segname)

    # add the table to the file
    elf.add_section(segname_tab)

    elf = elf.prepare(wordsize, endianess)
    # write the file
    elf.to_filename(options.target)

def seg_from_sec_script(sec, segments_to_sections_dict):
    """Goes through the dictionary of segment to section mappings from the 
    linker script and returns the segment that contains a given section
    """
    for segment in segments_to_sections_dict.keys():
        for script_sections in segments_to_sections_dict[segment]:
            if sec.name in [script_sections]:
                return segment

    
def score_segments(seg, segments_to_sections_dict):
    """ """
    seg_score = {}
    for sec in seg.get_sections():
        script_seg = seg_from_sec_script(sec, segments_to_sections_dict)
        if script_seg not in [seg_score.keys()]:
            seg_score[script_seg] = 0   
        seg_score[script_seg] += 1 
    return seg_score

def seg_from_script_guess(seg, segments_to_sections_dict):
    seg_score = score_segments(seg, segments_to_sections_dict)
    max_score = 0
    best_guess = "ERROR: NO GUESS"
    for scored_seg in seg_score.keys():
        if seg_score[scored_seg] > max_score:
            max_score = seg_score[scored_seg]
            best_guess = scored_seg
    
    return best_guess
        

def seg_list_rvct(elf, segments_to_sections_dict):
    seglist = []
    for seg in elf.segments:
        seg_name = seg_from_script_guess(seg, segments_to_sections_dict)
        seglist.append(seg_name)
    return seglist

def pad_non_loaded(elf, names):
    """
    Create a list of names for all segments in an ELF file from a list
    of names for the loadable segments.  Non-loadable segments are
    named ''.
    """
    names = names[:] # Make a copy to manipulate.
    seglist = []

    for seg in elf.segments:
        if seg.type == PT_LOAD:
            seglist.append(names[0])
            del names[0]
        else:
            seglist.append("")

    return seglist

def get_segment_names(options, elf, scripts, linker_name, orphans = []):
    """
    Return a list of names that correspond to the segments in the ELF
    file.

    The names can come from the command line, a file, the program
    headers listed in a linker script or generated from the flag
    values of the segments themselves.
    """
    seglist = []
    if len(scripts) != 0:
        scripts = scripts[0]
    

    # segments specifed in a file
    if options.file_segment_list:
        segfile = open(options.file_segment_list, 'r')

        seglist = pad_non_loaded(elf, segfile.readlines())

    # segments specified on command line
    elif options.cmd_segment_list:
        seglist = pad_non_loaded(elf, options.cmd_segment_list.split(r','))

    # get segments from linker script
    elif len(scripts) != 0 and linker_name == "rvct":
        segments_to_sections_dict = rvct_parser.extract_segment_names(scripts)
        # sections may appear in more than one segment
        # the correct segment in the linker script
        seglist = seg_list_rvct (elf, segments_to_sections_dict)

    # get segments from linker script
    elif len(scripts) != 0 and linker_name == "gcc":
        seglist = gnu_parser.extract_segment_names(scripts)


    # If no segment names are found, then generate default names.
    # Loadable segments are named according to their ELF flags.  Other
    # segments are unnamed because they are unused by elfweaver.
    if len(seglist) == 0:
        for seg in elf.segments:
            
            seg_name = ""
            if seg.has_sections():
               for sect in seg.get_sections():
                    if sect.name in orphans:
                        seg_name = sect.name

            if seg_name == "":
                if seg.flags & PF_R:
                    seg_name += 'r'

                if seg.flags & PF_W:
                    seg_name += 'w'

                if seg.flags & PF_X:
                    seg_name += 'x'

                if seg_name == "":
                    seg_name = "??"

            seglist.append(seg_name)

    return seglist

