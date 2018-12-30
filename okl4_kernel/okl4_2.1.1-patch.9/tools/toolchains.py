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

import copy
import os
import sys

class toolchain:
    def __init__(self, prefix="", suffix=""):
        self.dict = {}

        self.dict["TOOLPREFIX"] = prefix
        self.dict["TOOLSUFFIX"] = suffix
        self.dict["TOOLTYPE"] = ""

        self.dict["RANLIB"] = "${TOOLPREFIX}${_RANLIB}${TOOLSUFFIX}"
        self.dict["AR"] = "${TOOLPREFIX}${_AR}${TOOLSUFFIX}"
        self.dict["CC"] = "${TOOLPREFIX}${_CC}${TOOLSUFFIX}"
        self.dict["CXX"] = "${TOOLPREFIX}${_CXX}${TOOLSUFFIX}"
        self.dict["LINK"] = "${TOOLPREFIX}${_LINK}${TOOLSUFFIX}"
        self.dict["AS"] = "${TOOLPREFIX}${_AS}${TOOLSUFFIX}"
        self.dict["CPP"] = "${TOOLPREFIX}${_CPP}${TOOLSUFFIX}"
        self.dict["STRIP"] = "${TOOLPREFIX}${_STRIP}${TOOLSUFFIX}"
        self.dict["OBJCOPY"] = "${TOOLPREFIX}${_OBJCOPY}${TOOLSUFFIX}"

        self.dict["CCCOMSTR"] = "[CC  ] $TARGET"
        self.dict["CXXCOMSTR"] = "[CXX ] $TARGET"
            
        self.dict["ASPPCOMSTR"] = "[ASPP] $TARGET"
        self.dict["ASMCOMSTR"] = "[ASM ] $TARGET"
        self.dict["CPPCOMSTR"] = "[CPP ] $TARGET"        
        self.dict["LINKCOMSTR"] = "[LINK] $TARGET"        
        self.dict["ARCOMSTR"] = "[AR  ] $TARGET"        
        self.dict["RANLIBCOMSTR"] = "[LIB ] $TARGET"        
        
        self.dict["RANLIBFLAGS"] = "${_RANLIB_COM_FLAGS} ${RANLIB_FLAGS}"
        self.dict["_RANLIB_COM_FLAGS"] = []
        self.dict["RANLIB_FLAGS"] = []

        self.dict["ELFADORN"] = "./tools/pyelf/elfadorn"

        # This full AR command is part of default scons and not defined here
        #self.dict["ARFLAGS"] = ["${_AR_COM_FLAGS} ${AR_FLAGS}"]
        #self.dict["_AR_COM_FLAGS"] = []
        #self.dict["AR_FLAGS"] = ['r']

        self.dict["_CPPINCFLAGS"] = "$( ${_concat(INCPREFIX, CPPPATH, INCSUFFIX, __env__, RDirs, TARGET, SOURCE)} $)"
        self.dict["_CPPDEFFLAGS"] = "${_defines(CPPDEFPREFIX, CPPDEFINES, CPPDEFSUFFIX, __env__)}"

        ########################################################################
        # Setup default C compiler
        ########################################################################
        self.dict["_CC_COM_FLAGS"] = []
        self.dict["_CCFLAGS"] =  "$_CC_DEBUG $CC_STD_FLAGS $_CC_WARNINGS $CC_PLAT_FLAGS $_CC_OPTIMISATIONS  $CCFLAGS"

        self.dict["CC_PLAT_FLAGS"] = "${_platform(TOOLTYPE, 'c_flags')}"

        self.dict["CC_OPTIMISATIONPREFIX"] = "-O"
        self.dict["CC_OPTIMISATIONSUFFIX"] = ""
        self.dict["_CC_OPTIMISATIONS"] = "${_concat(CC_OPTIMISATIONPREFIX, CC_OPTIMISATIONS, CC_OPTIMISATIONSUFFIX, __env__)}"
        self.dict["CC_OPTIMISATIONS"] = ["2"]

        self.dict["CC_WARNINGPREFIX"] = "-W"
        self.dict["CC_WARNINGSUFFIX"] = ""
        self.dict["_CC_WARNINGS"] = "${_concat(CC_WARNINGPREFIX, CC_WARNINGS, CC_WARNINGSUFFIX, __env__)}"

        self.dict["CC_DEBUGPREFIX"] = "-g"
        self.dict["CC_DEBUGSUFFIX"] = ""
        self.dict["_CC_DEBUG"] = "${_concat(CC_DEBUGPREFIX, CC_DEBUG, CC_DEBUGSUFFIX, __env__)}"
        self.dict["CC_DEBUG"] = [""]

        self.dict["CCCOM"] =  "$CC $_CC_COM_FLAGS $_CCFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS $SOURCES -c -o $TARGET"

        ########################################################################
        # Setup default C++ compiler
        ########################################################################
        self.dict["_CXX_COM_FLAGS"] = []
        self.dict["_CXXFLAGS"] = "$_CXX_DEBUG $_CXX_WARNINGS $CXX_PLAT_FLAGS $_CXX_OPTIMISATIONS $CXXFLAGS"

        self.dict["CXX_PLAT_FLAGS"] = "${_platform(TOOLTYPE, 'cxx_flags')}"
        
        self.dict["CXX_OPTIMISATIONPREFIX"] = "-O"
        self.dict["CXX_OPTIMISATIONSUFFIX"] = ""
        self.dict["_CXX_OPTIMISATIONS"] = "${_concat(CXX_OPTIMISATIONPREFIX, CXX_OPTIMISATIONS, CXX_OPTIMISATIONSUFFIX, __env__)}"
        self.dict["CXX_OPTIMISATIONS"] = ["2"]

        self.dict["CXX_WARNINGPREFIX"] = "-W"
        self.dict["CXX_WARNINGSUFFIX"] = ""
        self.dict["_CXX_WARNINGS"] = "${_concat(CXX_WARNINGPREFIX, CXX_WARNINGS, CXX_WARNINGSUFFIX, __env__)}"

        self.dict["CXX_DEBUGPREFIX"] = "-g"
        self.dict["CXX_DEBUGSUFFIX"] = ""
        self.dict["_CXX_DEBUG"] = "${_concat(CXX_DEBUGPREFIX, CXX_DEBUG, CXX_DEBUGSUFFIX, __env__)}"
        self.dict["CXX_DEBUG"] = [""]

        self.dict["CXXCOM"] = "$CXX $_CXX_COM_FLAGS $_CXXFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS $SOURCES -c -o $TARGET"

        ########################################################################
        # Setup default assembler
        ########################################################################
        self.dict["_AS_COM_FLAGS"] = []
        self.dict["ASCOM"] = "$AS $_AS_COM_FLAGS $AS_PLAT_FLAGS $ASFLAGS -o $TARGET $SOURCES"
        self.dict["AS_PLAT_FLAGS"] = "${_platform(TOOLTYPE, 'as_flags')}"

        # Setup linker
        self.dict["_LINK_COM_FLAGS"] = []
        self.dict["UNADORNED_LINKCOM"] = "$LINK $_LINK_COM_FLAGS $LINK_PLAT_FLAGS $_LINKADDRESS $_LINKSCRIPTS $LINKFLAGS $CRTOBJECT $SOURCES $_LIBFLAGS -o $TARGET"
        self.dict["LINKCOM"] = "$ELFADORN -s -o $TARGET $ELFADORN_FLAGS   -- $UNADORNED_LINKCOM"

        self.dict["LINK_PLAT_FLAGS"] = "${_platform(TOOLTYPE, 'link_flags')}"

        self.dict["_LINKADDRESS"] = "${_linkaddress(LINKADDR_PREFIX, TARGET.LINKFILE)}"

        self.dict["_LINKSCRIPTS"] = "${_concat(LINK_SCRIPT_PREFIX, LINKSCRIPTS, LINK_SCRIPT_SUFFIX, __env__, _as_string)}"
        self.dict["LINKSCRIPTS"] = []
        self.dict["LINK_SCRIPT_SUFFIX"] = ""
        self.dict["LINK_SCRIPT_PREFIX"] = ""

        self.dict["LIBPREFIX"] = "lib"
        self.dict["LIBSUFFIX"] = ".a"

        self.dict["_LIBDIRFLAGS"] = "$( ${_concat(LIBDIRPREFIX, LIBPATH, LIBDIRSUFFIX, __env__, RDirs)} $)"
        self.dict["LIBDIRPREFIX"] = "-L"
        self.dict["LIBDIRSUFFIX"] = ""

        # Setup C-preprocessor
        self.dict["_CPP_COM_FLAGS"] = []
        self.dict["_CPP_STDIN_EXTRA_FLAGS"] = [ "-" ]
        self.dict["CPPCOM"] =    "$CPP $_CPP_COM_FLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS $SOURCES -o $TARGET"

        self.dict["CPPPATH"] = []
        self.dict["CPPDEFINES"] = []

class arm_toolchain(toolchain):
    def __init__(self):
        toolchain.__init__(self)
        self.type = "rvct"
        
        self.dict["TOOLTYPE"] = "rvct"
        
        self.dict["_RANLIB"] = "armar"
        self.dict["_RANLIB_COM_FLAGS"] = ["-s"]
        self.dict["_AR"] = "armar"
        self.dict["_CC"] = "armcc"
        self.dict["_CXX"] = "armcpp"
        self.dict["_LINK"] = "armlink"
        self.dict["_AS"] = "armasm"
        self.dict["_CPP"] = "armcc"
        self.dict["_CPP_COM_FLAGS"] = ["-E"]

        self.dict["_LIBFLAGS"] = "${_concat('', LIBS, '',  __env__, _findlib)}"

        self.dict["ASMCOM"] = "$CXXCOM -S"

        self.dict["LINK_SCRIPT_PREFIX"] = "--scatter "

	# This command is ugly, but this is whatthe ARM manual suggests
	self.dict["ASPPCOM"] = "$CC -DASSEMBLY -D__ASSEMBLER__ $ASPPFLAGS $CXX_PLAT_FLAGS $CPPFLAGS $_CPPDEFFLAGS "\
                               "$_CPPINCFLAGS -E -o ${TARGET}.s $SOURCES && $AS $ASFLAGS -o $TARGET ${TARGET}.s"

class ads_toolchain(arm_toolchain):
    def __init__(self):
        arm_toolchain.__init__(self)
        
        self.name = "ads"

        self.dict["TOOLTYPE"] = "ads"
        if sys.version_info[:2] >= (2, 4):
            self.dict["TOOLPREFIX"] = os.path.join(os.getcwd(), "tools/rvds/ads_")
        else:
            self.dict["TOOLPREFIX"] = "/opt/arm/linux/bin/"

        self.dict["LINKFLAGS"] = []
        self.dict["LINKADDR_PREFIX"] = "-ro 0x"
        gen_flags = ["-apcs", "/nointerwork", "-g", "-Dinline=__inline", "-Wb"]
        self.dict["CC_OPTIMISATIONS"] = ["2"]
        self.dict["CXX_OPTIMISATIONS"] = ["2"]
        self.dict["CCFLAGS"] = copy.copy(gen_flags)
        self.dict["CXXFLAGS"] = copy.copy(gen_flags)
        self.dict["ASFLAGS"] = ["-apcs", "/nointerwork",  "-Ilibs/l4/include/l4/arm", "$AS_PLAT_FLAGS"]
        self.dict["CPPFLAGS"] = ["-D__ADS__"]
        self.dict["CCFLAGS"] += ["-D__ADS__"]
        self.dict["CXXFLAGS"] += ["-D__ADS__"]            
        self.dict["ASPPFLAGS"] = ["-D__ADS__"]            

ads_toolchain = ads_toolchain()

class rvct_toolchain(arm_toolchain):
    def __init__(self, gnu=False, *args):
        arm_toolchain.__init__(self, *args)
        self.type = "rvct"

        if sys.version_info[:2] >= (2, 4):
            self.dict["TOOLPREFIX"] = os.path.join(os.getcwd(), "tools/rvds/rvct_")
        else:
            self.dict["TOOLPREFIX"] = "/opt/rvds/RVCT/Programs/2.2/503/linux-pentium/"

        self.dict["LINKFLAGS"] = ["--noremove", "--datacompressor=off"]
        self.dict["LINKADDR_PREFIX"] = "--ro 0x"
        gen_flags = ["--apcs", "/nointerwork", "-g", "-Dinline=__inline", "-D__ARMEL__", "-W"]
        self.dict["CCFLAGS"] = copy.copy(gen_flags)
        self.dict["CXXFLAGS"] = copy.copy(gen_flags)
        self.dict["ASFLAGS"] = ["--apcs", "/interwork", "-Ilibs/l4/include/l4/arm", "$AS_PLAT_FLAGS"]
        self.dict["CC_OPTIMISATIONS"] = ["2"]
        self.dict["CXX_OPTIMISATIONS"] = ["2"]
        if gnu:
            self.name = "rvct_gnu"
            self.dict["CPPFLAGS"] = ["--gnu", "-U__GNUC__", "-D__RVCT_GNU__"]
            self.dict["CCFLAGS"] += ["--gnu", "-U__GNUC__", "-D__RVCT_GNU__"]
            self.dict["CXXFLAGS"] += ["--gnu", "-U__GNUC__", "-D__RVCT_GNU__"]            
            self.dict["ASPPFLAGS"] = ["--gnu", "-U__GNUC__", "-D__RVCT_GNU__"]            
        else:
            self.name = "rvct"
            self.dict["CPPFLAGS"] = ["-D__RVCT__"]
            self.dict["CCFLAGS"] += ["-D__RVCT__"]
            self.dict["CXXFLAGS"] += ["-D__RVCT__"]            
            self.dict["ASPPFLAGS"] = ["-D__RVCT__"]            

rvct_gnu_toolchain = rvct_toolchain(gnu=True)
rvct_toolchain = rvct_toolchain()


class generic_gcc(toolchain):
    def __init__(self, *args):
        toolchain.__init__(self, *args)
        self.name = "gnu"
        self.type = "gnu"

        self.dict["TOOLTYPE"] = "gnu"

        self.dict["_RANLIB"] = "ranlib"
        self.dict["_AR"] = "ar"
        self.dict["_CC"] = "gcc"
        self.dict["_CXX"] = "g++"
        self.dict["_CPP"] = "gcc"
	self.dict["_STRIP"] = "strip"
        self.dict["_CPP_COM_FLAGS"] = ["-E", "-P", "-x", "c"]
        self.dict["_LINK"] = "ld"
        self.dict["_STRIP"] = "strip"
        self.dict["_OBJCOPY"] = "objcopy"

        self.dict["LINKFLAGS"] = []

        self.dict["CCFLAGS"] = ["-nostdlib", "-nostdinc", "-g"]
        self.dict["CXXFLAGS"] = ["-nostdlib", "-nostdinc", "-fno-builtin", "-g",
                                 "-fno-exceptions", "-fomit-frame-pointer",
                                 "-finline-limit=99999999"]

        # This is the C compiler command to use
        self.dict["CC_STD"] = "gnu99"
        self.dict["CC_STD_FLAGS"] = "--std=$CC_STD"

        self.dict["CC_WARNINGS"] = ["all", "strict-prototypes", "missing-prototypes",
                                    "nested-externs", "missing-declarations",
                                    "redundant-decls", "undef", "pointer-arith",
                                    "no-nonnull", "error"]
        self.dict["CXX_WARNINGS"] = ["all", "redundant-decls", "undef", "pointer-arith",
                                     "no-uninitialized", "error"]
        self.dict["LIBGCC"] = "-lgcc"

        self.dict["_LIBFLAGS"] = "$_LIBDIRFLAGS --start-group $LIBGCC "\
                                 "${_stripixes(LIBLINKPREFIX, LIBS, LIBLINKSUFFIX, LIBPREFIX, LIBSUFFIX, __env__)} --end-group"
        
        self.dict["LINK_SCRIPT_PREFIX"] = "-T"
        self.dict["LINKADDR_PREFIX"] = "-Ttext="
        
        self.dict["ASPPCOM"] = "$CC -x assembler-with-cpp -DASSEMBLY $AS_PLAT_FLAGS $ASPPFLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS -c -o $TARGET $SOURCES"

        self.dict["ASMCOM"] = "$CC -S $CC_PLAT_FLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS -c -o $TARGET $SOURCES"

# GCC has a version of at least 3.4
class generic_gcc_3_4(generic_gcc):
    def __init__(self, *args):
        generic_gcc.__init__(self, *args)

        # Force gcc to use inling in large functions, even if it is going to
        # cause the large functions to be larger. Supported only on gcc >= 3.4.
        self.dict["CXXFLAGS"].extend(["--param",  "large-function-growth=1000"])

# use this if you want to use GCC's own libgcc
class generic_gcc_3_4_libgcc(generic_gcc_3_4):
    def __init__(self, *args):
        generic_gcc_3_4.__init__(self, *args)
        self.dict["LIBGCC"] = "${_libgcc(__env__)}"

class generic_gcc_400(generic_gcc):
    def __init__(self, *args):
        generic_gcc.__init__(self, *args)
        self.dict["_AR"] = "ar"
        self.dict["_CC"] = "gcc-4.0"
        self.dict["_CXX"] = "g++-4.0"
        self.dict["_CPP"] = "gcc-4.0"

class x86_64_toolchain(generic_gcc):
    def __init__(self, *args):
        generic_gcc.__init__(self, *args)
        self.dict["_AR"] = "ar"
        self.dict["_CC"] = "gcc-4.0"
        self.dict["_CXX"] = "g++-4.0"
        self.dict["_CPP"] = "gcc-4.0"
        self.dict["LINK_PLAT_FLAGS"] = ["-static", "-O2", "-melf_x86_64", "-n", "-L/usr/lib/gcc/x86_64-linux-gnu/4.0.3/", "-v"]
        self.dict["LINKFLAGS"] = []
        self.dict["_LIBFLAGS"] = "--start-group ${_concat('', LIBS, '',  __env__, _findlib)} --end-group"
        array = ["-mcmodel=kernel", "-m64", "-march=k8", "-Wformat"]
        self.dict["CCFLAGS"] += array 
        self.dict["CXXFLAGS"] += array + ["-fno-rtti"]
        self.dict["ASFLAGS"] = array
        self.dict["ASPPFLAGS"] = array
        self.dict["CC_PLAT_FLAGS"] = ["-m64"]

# ia32 compiler for Apple Macho toolchains
class macho_ia32_toolchain(generic_gcc):
    def __init__(self, *args):
        generic_gcc.__init__(self, *args)
        self.dict["LINK_PLAT_FLAGS"] = ["-all_load", "-static", "-arch", "i386", "-e", "_start"]
        self.dict["LINKFLAGS"] = []
        self.dict["_LIBFLAGS"] = "$LIBS"
        self.dict["_LIBFLAGS"] = "$_LIBDIRFLAGS  " \
                                 "${_stripixes(LIBLINKPREFIX, LIBS, LIBLINKSUFFIX, LIBPREFIX, LIBSUFFIX, __env__)}"
        array = ["-arch", "i386", "-fno-leading-underscore", "-O2", "-fno-builtin-printf"]
        self.dict["CCFLAGS"] += array + ["-fno-pic"]
        self.dict["CXXFLAGS"] += array + ["-fno-pic", "-fno-rtti"]
        self.dict["ASFLAGS"] = array
        self.dict["ASPPFLAGS"] = array

        # The weirdest bit about macho is the linker...
        self.dict["_LINK"] = "ld"
        self.dict["_LINK_COM_FLAGS"] = []
        self.dict["LINKCOM"] = "$LINK $_LINK_COM_FLAGS $LINK_PLAT_FLAGS $_LINKADDRESS $_LINKSCRIPTS $LINKFLAGS $CRTOBJECT $SOURCES $_LIBFLAGS -o $TARGET"

        # self.dict["LINK_PLAT_FLAGS"] = "${_platform(TOOLTYPE, 'link_flags')}"

        # self.dict["_LINKADDRESS"] = "${_linkaddress(LINKADDR_PREFIX, TARGET.LINKFILE)}"
        self.dict["_LINKADDRESS"] = ""

        # proper link script processing doesn't work. We set L4's to get the
        # dependency.
        self.dict["_LINKSCRIPTS"] = "${_concat(LINK_SCRIPT_PREFIX, LINKSCRIPTS, LINK_SCRIPT_SUFFIX, __env__, _as_string)}"
        self.dict["_LINKSCRIPTS"] = ""
        self.dict["LINKSCRIPTS"] = []
        self.dict["LINK_SCRIPT_SUFFIX"] = ""
        self.dict["LINK_SCRIPT_PREFIX"] = ""


#gnu_arm_libgcc_toolchain = generic_gcc("arm-linux-")
gnu_arm_toolchain = generic_gcc_3_4_libgcc("arm-linux-")
gnu_arm_nptl_toolchain = generic_gcc_3_4_libgcc("/opt/okl/Linux-i386/stow/arm-linux/bin/arm-linux-")
gnu_mips64_toolchain = generic_gcc_3_4("mips64-elf-")
gnu_mips32_toolchain = generic_gcc_3_4("mips-linux-")
arm_linux_toolchain = generic_gcc_3_4("/opt/tools/3.4/arm-linux/bin/arm-linux-")
gnu_ia32_toolchain = generic_gcc("i686-unknown-linux-gnu-")
gnu_ia32_nptl_toolchain = generic_gcc("/opt/okl/Linux-i386/stow/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-")

############################################################################
# Import customer specific toolchains
############################################################################
import os
for thedir in ["arch", "platform", "cust"]:
    if os.path.exists(thedir):
        for dir_ in os.listdir(thedir):
            file_name = os.path.join(thedir, dir_, "tools", "toolchains.py")
            if os.path.exists(file_name):
                execfile(file_name)

for thedir in ["tools/cust"]:
    if os.path.exists(thedir):
        for dir_ in os.listdir(os.path.join(thedir)):
            file_name = os.path.join(thedir, dir_, "toolchains.py")
            if os.path.exists(file_name):
                execfile(file_name)
