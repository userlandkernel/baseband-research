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
import os

"""
This file contains functions to parse the GNU LD command line arguments
(as much as we need to understand anyway)
"""

def get_linker_name(args, linker_command):
    linker = None
    for arg in args:
	if arg.endswith("rvct_armlink"):
	    return "rvct"
        if arg.startswith("--script="):
            return "gcc"
        elif arg.startswith("-T") and len(arg) > 2:
            if not(arg.startswith("-Tbss") or arg.startswith("-Tdata") or arg.startswith("-Ttext")):
                return "gcc"


        if arg == "-T":
            return "gcc"
        elif arg == "--scatter":
            return "rvct"

    ret  = os.spawnvp(os.P_WAIT, linker_command, [ linker_command, "--version"])
    if ret == 0:
        return "gcc"
    else:
        return "rvct"


   


def get_script_names(args):
    scripts = []
    next = False
    for arg in args:
        if next:
            scripts.append(arg)
            next = False
        if arg.startswith("--script="):
            arg = arg[9:]
            scripts.append(arg)
        elif arg == "-T":
            next = True
        elif arg.startswith("-T") and len(arg) > 2:
            if not(arg.startswith("-Tbss") or arg.startswith("-Tdata") or arg.startswith("-Ttext")):
                arg = arg[2:]
                scripts.append(arg)
        elif arg == "--scatter":
            next = True

    return scripts

# ld arguments that take an argument themselves, that are seperated by a space
space_arg = [
    "-b",
    "-c",
    "-e",
    "--exclude-libs",
    "-f", "--auxiliary",
    "-F", "--filter",
    "-fini",
    "-init",
    "-o",
    "-O",
    "-R",
    "-u",
    "-T",
    "-y",
    "-Y",
    "-z",
    "-assert",
    "--defsym",
    "--dynamic-linker",
    "-Map",
    "--oformat",
    "--retain-symbols-file",
    "-rpath",
    "-rpath-link",
    "--sort-section",
    "--split-by-file",
    "--split-by-reloc",
    "--section-start",
    "-Tbss",
    "-Tdata",
    "-Ttext",
    "--ro",
    "-ro",
    "--wrap"]
   

def remove_arguments(args):
    ignore = False
    leftovers = []

    for arg in args:
        if not arg.startswith("-") and not ignore:
            leftovers.append(arg)
        if not ignore and arg in space_arg:
            ignore = True
        else:
            ignore = False

    return leftovers

# These are other ld arguments, no current use of these lists, so they are left
# commented out

# sigle dash args with arg but no space between
#eg Llibpath
#jam_arg = [["-a"],
#    ["-A"],
#    ["-G"],
#    ["-h"],
#    ["-l"],
#    ["-L"],
#    ["-m"]]

# = argument
# = argument may be optional
#key_arg = [["--unique"],
#    ["--architecture"],
#    ["--format"],
#    ["--mri-script"],
#    ["--entry"],
#    ["--gpsize"],
#    ["-soname"],
#    ["--library"],
#    ["--library-path"],
#    ["--output"],
#    ["--just-symbols"],
#    ["--undefined"],
#    ["--trace-symbol"],
#    ["--sysroot"],
#    ["--unresolved-symbols"],
#    ["--version-script"],
#    ["--hash-size"],
#    ["--script"],
#    ["--demangle"]]

# no arg options
#no_args = [ ["-d", "-dc", "-dp"],
#    ["-E", "--export-dynamic"],
#    ["-EB"],
#    ["-EL"],
#    ["-g"],
#    ["-i"],
#    ["-M", "--print-map"],
#    ["-n", "--nmagic"],
#    ["-N", "--omagic"],
#    ["--no-omagic"],
#    ["-q", "--emit-relocs"],
#    ["--force-dynamic"],
#    ["-r", "--relocatable"],
#    ["-s", "--strip-all"],
#    ["-S", "--strip-debug"],
#    ["-t", "--trace"],
#    ["-Ur"],
#    ["-v", "-V", "--version"],
#    ["-x", "--discard-all"],
#    ["-X", "--discard-locals"],
#    ["-y", "--trace-symbol"],
#    ["-(", "--start-group"],
#    ["-)", "--end-group"],
#    ["--accept-unknown-input-arch"],
#    ["--no-accept-unknown-input-arch"],
#    ["--as-needed"],
#    ["--no-as-needed"],
#    ["--add-needed"],
#    ["--no-add-needed"],
#    ["-Bdynamic", "-dy", "-call-shared"],
#    ["-Bgroup"],
#    ["-Bstatic", "-dn", "-non-shared", "-static"],
#    ["-Bsymbolic"],
#    ["--check-sections"],
#    ["--no-check-sections"],
#    ["--cref"],
#    ["--no-define-common"],
#    ["--no-demangle"],
#    ["--fatal-warnings"],
#    ["--force-exe-suffix"],
#    ["--no-gc-sections"],
#    ["--gc-sections"],
#    ["--help"],
#    ["--target-help"],
#    ["--no-keep-memory"],
#    ["--no-undefined"],
#    ["--allow-multiple-definition"],
#    ["--allow-sh-lib-undefined"],
#    ["--no-allow-sh-lib-undefined"],
#    ["--no-undefined-version"],
#    ["--default-symver"],
#    ["--default-imported-symver"],
#    ["--no-warn-mismatch"],
#    ["--no-whole-archive"],
#    ["--noinhibit-exec"],
#    ["-nostdlib"],
#    ["-pie", "--pic-executable"],
#    ["-qmagic"],
#    ["-Qy"],
#    ["--relax"],
#    ["-shared", "-Bshareable"],
#    ["--sort-common"],
#    ["--stats"],
#    ["--traditional-format"],
#    ["--dll-verbose", "--verbose"],
#    ["--warn-common"],
#    ["--warn-constructors"],
#    ["--warn-multiple-gp"],
#    ["--warn-once"],
#    ["--warn-section-align"],
#    ["--warn-shared-textrel"],
#    ["--warn-unresolved-symbols"],
#    ["--error-unresolved-symbols"],
#    ["--whole-archive"],
#    ["--eh-frame-hdr"],
#    ["--enable-new-dtags"],
#    ["--disable-new-dtags"],
#    ["--reduce-memory-overheads"]]


