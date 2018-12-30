#!/usr/bin/env python2.4
"""
This file contains the main implementation of the build system.
"""
#####################################################################
# Copyright (c) 2006, National ICT Australia
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
#####################################################################

if __name__ == "__main__":
    # This is the code that is executed when build.py is called as
    # as an executable, either through ./tools/build.py or
    # python tools/build.py

    # This is a little bit magic. This is the first entry point into
    # the build and we use it to kick of SCons, which does most of the
    # work.

    # SCons later reloads this script to execute the build system proper,
    # but in that case this code is not executed.

    import sys
    # Manipulate the system path so we can import SCons.Script
    sys.path = ["tools"] + sys.path

    import SCons.Script
    import os

    # We want to use -f, to force SCons to read tools/build.py as the
    # main SConscript. So we first make sure the user isn't trying
    # to use "-f" as well.
    for arg in sys.argv:
        if arg.startswith("-f"):
            print >> sys.stderr, "scons.py does not support the use of '-f'. " \
                  "Please use project=<projectname> syntax"
            sys.exit(1)

    # Append -f so that tools/build.py is used as top
    # level SConstruct
    sys.argv.append("-ftools%sbuild.py" % os.path.sep)

    # FIXME: We should be able to enable this!
    # sys.argv.append("--warn=all")

    # Run the main script
    SCons.Script.main()

    # Just check we don't fall off the end.
    raise Exception("Must not get here")

######################################################################
# Library Import + Version checking
######################################################################

import sys
sys.path.append("tools") # Allow us to import bootimg.py
sys.path.append("tools/pyelf")
sys.path = ["tools/pexpect"] + sys.path # Allow us to import pexpect.py

import SCons.Errors, os.path
import SCons.Defaults
import SCons.Scanner
import os, glob, copy, string, shutil, stat
from stat import ST_SIZE
import traceback
from cStringIO import StringIO
from types import *
from util import *
import sets
from elf.core import PreparedElfFile
from elf.constants import PF_R, PF_W, PT_LOAD
from elf.util import align_up, align_down
import pexpect
import time
from xml.dom import minidom
from optparse import OptionParser

# Make it easier to raise UserErrors
UserError = SCons.Errors.UserError

# We want the SConsign file (where it stores
# information on hashs and times
SConsignFile(".sconsign")

# Work make the scons version easy to test
scons_version = tuple([int(x) for x in SCons.__version__.split(".")])
Export("scons_version")
Export("UserError")

# Make sure that we have a decent version of SCons
if scons_version <= (0, 95):
    raise UserError, "Support for SCons 0.95 has been dropped. " + \
          "Please upgrade to at least 0.96"

# We check we have at least version 2.3, If we don't we are in trouble!
if sys.version_info < (2, 3):
    raise UserError, "To use the Kenge build system you need Python2.3 "  + \
          "including the python devel packages."

def src_glob(search, replacement = {}):
    """Src glob is used to find source files easily e.g: src_glob("src/*.c"),
    the reason we can't just use glob is due to the way SCons handles out
    of directory builds."""
    if search.startswith('#'):
        # what does '#' signify?
        search = search[1:]
        dir = os.path.dirname(search)
        if dir != "":
            dir = '#' + dir + os.sep
            #is the below any different from Dir('.').srcnode.abspath ??
        src_path = Dir('#').srcnode().abspath
    elif search.startswith(os.sep):
        # we have been given an absolute path
        dir = os.path.dirname(search)
        if dir != "":
            dir = dir + os.sep
        src_path = os.sep
    else:
        dir = os.path.dirname(search)
        if dir != "":
            dir = dir + os.sep
        src_path = Dir('.').srcnode().abspath

    src_path = src_path % replacement
    search = search % replacement
    files = glob.glob(src_path + os.sep + search)
    files = map(os.path.basename, files)
    ret_files = []
    for file in files:
        ret_files.append(dir + file)
    # Sort the files so the linkers get the same list of objects
    # and hence deterministic builds
    ret_files.sort()
    return ret_files

Export("src_glob")

##########################################################################
# Get machine information
##########################################################################
from toolchains import *

import machines
available_machines = []
for each in dir(machines):
    attr = getattr(machines, each)
    if type(attr) == ClassType and \
           issubclass(attr, machines.Machine) and \
           not attr.virtual:
        available_machines.append(attr)

# Make all the machines globally accessable
from machines import *
Export("machines")

################################################################################
# Read configuration files
################################################################################

class Options:
    """Class to hold a set of configuration options. This works as a
    recursive dictionary, with access through the "." method. This allows
    settings things such as:

    conf = Options()
    conf.FOO.BAR.BAZ = 1

    And all intermediate level will be created automagically."""
    
    def __init__(self):
        """Create a new instance."""
        self.__dict__["dict"] = {}
    
    def __getattr__(self, attr):
        """Magic getattr method to implement accessing dictionary
        members through the . syntax. Will create a new Options dictionary
        when accessing new names. This method is only called in cases
        when the attribute doesn't already exist."""
        if attr.startswith("__"): # Disallow access to "__" names
            raise AttributeError
        self.__dict__[attr] = Options()
        return self.__dict__[attr]

    def __setattr__(self, attr, value):
        """Sets a new value in the dictionary"""
        self.dict[attr.upper()] = value

conf = Options()
if os.path.exists(".conf"):
    execfile(".conf", globals())

for (key, value) in ARGUMENTS.items():
    foo = "conf.%s=%s" % (key, value)
    alt = "conf.%s='%s'" % (key, value)
    try:
        exec(foo, globals())
    except NameError:
        exec(alt, globals())
    except SyntaxError:
        exec(alt, globals())
    except:
        raise UserError, ("Couldn't parse value <%s> passed to argument %s. " + \
              "Maybe you forgot some quotes?") % (value, key)
Export("conf")

############################################################################
# Cleaning stuff
############################################################################

# Determine if the user is trying to clean
cleaning = contains_any(["--clean", "--remove", "-c"], sys.argv)

# Clean out any .pyc from tool
if cleaning:
    pyc_clean("tools")

class KengeBuild:
    """Top level class in the hierarchy of instantiated classes. This
    represents the top of the build."""
    def __init__(self, **kargs):
        # Any default arguments from the SConstruct are overridden by the user
        kargs.update(conf.dict)

        def handle_arg(name, default):
            ret = kargs.get(name, default)
            if name in kargs: del kargs[name]
            return ret

        self.build_dir = handle_arg("BUILD_DIR", "#build")
        if not self.build_dir.startswith('#'):
            self.build_dir = '#' + self.build_dir

        # Here we setup the sconsign file. This is the database in which
        # SCons holds file signatures. (See man scons for more information)
        if not os.path.exists(Dir(self.build_dir).abspath):
            os.mkdir(Dir(self.build_dir).abspath)
        SConsignFile(Dir(self.build_dir).abspath + os.sep + "sconsign")

        self.machine = handle_arg("MACHINE", None)

        if self.machine is None:
            raise UserError, "You must specify the MACHINE you " + \
                  "wish to build for."

        if type(self.machine) == type(""):
            raise UserError, "%s is not a valid machine, please specify a machine from tools/machines.py" % self.machine
        self.toolchain = handle_arg("TOOLCHAIN", self.machine.default_toolchain)

        if self.toolchain is None:
            raise UserError, "No TOOLCHAIN specified. You did not specify " + \
                  "a toolchain, and the machine you chose \n\tdoes not " + \
                  "have a default one."

        self.image_type = handle_arg("IMAGE_TYPE", "generic")
        
        # Any remaining options are saved, as they may be passed down to
        # KengeEnvironment's created.

        self.extra_args = kargs

        # Initialise the expected test output list.
        self.expect_test_data = []


    def KengeEnvironment(self, name, *args, **kargs):
        """Create a new environment in this build.
        Name is the name of the environment to create.
        *args are passed on to the KengeEnvironment class.
        **kargs are any options configuration options."""
        # Create a copy of our extra args, and then update first
        # with any overriders from the kargs, and then from the user's
        # conf
        new_args = copy.copy(self.extra_args)
        new_args.update(kargs)
        
        # If the user has specified any <name>.<foo> options pass
        # them through.
        if hasattr(conf, name):
            new_args.update(getattr(conf, name).dict)

        # Finally update these required fields, with sane defaults,
        # only if they haven't already been overridden.
        new_args["SYSTEM"] = new_args.get("SYSTEM", name)
        new_args["CUST"] = new_args.get("CUST", self.machine.cust)
        new_args["MACHINE"] = new_args.get("MACHINE", self.machine)
        new_args["TOOLCHAIN"] = new_args.get("TOOLCHAIN", self.toolchain)
        new_args["BUILD_DIR"] = new_args.get("BUILD_DIR", self.build_dir +
                                             os.sep + name)

        return KengeEnvironment(name, *args, **new_args)



       

    def get_run_method(self):
        if len(self.machine.run_methods.keys()) > 1:
            key = conf.dict.get('RUN', self.machine.default_method)
        else:
            key = self.machine.run_methods.keys()[0]
        return self.machine.run_methods[key]

    def RunImage(self, run_image):
        def simulate(target, source, env):
            run_method = self.get_run_method()
            cmdline = run_method(target, source, self.machine)
            os.system(cmdline)

        Command("simulate", run_image,
                Action(simulate, "[SIM ] $SOURCE"))
        
        def mem_stats(target, source, env):
            cmdline = "tools/memory-usage %s" % self.build_dir[1:]
            os.system(cmdline)
            
        Command("mem_stats", run_image,
                Action(mem_stats, "$SOURCE"))

    def test(self, target, source, env):
        def print_log(log):
            print '--- SIMULATOR LOG ' + '-'*60
            print log.getvalue()
            print '-'*78

        def print_coverage(cov_stdout, tracelog_file):
            if str(target[0]) == "coverage":
                for line in cov_stdout:
                    if str(line) == "</summary>\n": print line
                    else: print line,
                os.unlink(tracelog_file)

        # If coverage target is specified, run test code coverage script in
        # parallel with skyeye
        cov_stdout = ""
        tracelog_file = ""
        if str(target[0]) == "coverage":
            run_arg = conf.dict.get('RUN', self.machine.default_method)
            if run_arg != "skyeye":
                raise UserError, "Can not run target \"coverage\" with " + \
                                 "the specified run method \"%s\"." % run_arg + \
                                 "\nThe target \"coverage\" currently supports " + \
                                 "only skyeye simulator. Stop." 
            try:
                self.machine.skyeye += ".tracelog"
            except:
                raise UserError, "The specified machine does not support " + \
                                 "target \"coverage\".\nThe target \"coverage\" " + \
                                 "currently supports only skyeye simulator. Stop."
            f = open("tools/sim_config/" + self.machine.skyeye, "r")
            tracelog_content = f.read()
            m = re.search('logfile=(?P<logfile>[^\s(,\s)]*)', tracelog_content)
            f.close()
            if m == None:
                raise Error, "Can not find \"logon\" option in Skyeye configuration file."
            tracelog_file = m.group('logfile')
            self.test_libs = conf.dict.get("TEST_LIBS", "")
            if project == "iguana":
                if "all" in self.test_libs:
                    packages_to_assess = "iguana/server"
                    for ldir in os.listdir("libs"):
                        packages_to_assess += "," + os.path.join("libs", ldir)
                else:
                    packages_to_assess = re.sub(r'(^|,){1}', r'\1libs/', self.test_libs)
            elif project == "l4test":
                packages_to_assess = "pistachio,l4test"
            else:
                raise UserError, "Do not know how to make target \"coverage\" for project %s" % project
            if os.path.exists(tracelog_file):
                os.unlink(tracelog_file)
            os.mkfifo(tracelog_file)
            cov_stdout = os.popen("python tools/coverage.py -p %s -b %s -t %s -s" % (packages_to_assess, Dir(self.build_dir), tracelog_file), 'r')

        # Get the testing value
        timeout = float(conf.TESTING.dict.get("TIMEOUT", 30.0))
        run_method = self.get_run_method()
        cmdline = run_method(target, source, self.machine)
        if len(self.expect_test_data) == 0:
            raise UserError, "No expected test output supplied " + \
                    "for this build"
        log = StringIO()
        if pexpect.__version__ >= '2.0':
            tester = pexpect.spawn(cmdline, timeout=timeout, logfile=log)
        else:
            tester = pexpect.spawn(cmdline, timeout=timeout)
            tester.setlog(log)
        for in_, out_ in self.expect_test_data:
            try:
                i = tester.expect(in_)
            except pexpect.ExceptionPexpect, e:
                if isinstance(e, pexpect.TIMEOUT):
                    print "Timed out waiting for: <%s>" % in_
                elif isinstance(e, pexpect.EOF):
                    print "Simulator exited while waiting for: <%s>" % in_

                print_log(log)
                tester.terminate(True)
                print_coverage(cov_stdout, tracelog_file)
                raise UserError, "Failed test."
            if i >= 1:
                print_log(log)
                tester.terminate(True)
                print_coverage(cov_stdout, tracelog_file)
                raise UserError, "Failed test, matched <%s>" % in_[i]
            if not out_ is None:
                for char in out_:
                    time.sleep(0.2)
                    tester.send(char)
                tester.send('\n')
                #tester.sendline(out_)
        tester.terminate(True)
        if conf.TESTING.dict.get("PRINT_LOG", False):
            print_log(log)
        print_coverage(cov_stdout, tracelog_file)

    def TestImage(self, test_image):
        Command("simulate_test", test_image,
                Action(self.test, "[TEST] $SOURCE"))

    def CoverageImage(self, cov_image):
        Command("coverage", cov_image,
                Action(self.test, "[COV ] $SOURCE"))

    # Layout Macho files by doing a relocation link
    def MachoLayoutVirtual( self, apps ):

        kernel = apps[0]
        apps = apps[1:]
        reloc_apps = []

        # default base address
        base_addr = 0x00d00000

        # Create a relocation for each app
        for app in apps:
            rtarget = Flatten( app )[0].abspath + ".reloc"
            print "rtarget is %s" % rtarget
            rcmd = "ld -image_base %s -o $TARGET $SOURCE" % hex(base_addr)
            print "rcmd is '%s'" % rcmd
            rapp =  Command( rtarget,
                             app,
                             rcmd )

            reloc_apps += rapp

            # XXX: could do this better, but too lazy right now
            base_addr += 0x100000

        return kernel + reloc_apps

    # Mach-O binaries are bundled with the 'legion' utility
    def CreateLegionImage(self, apps):

        # actual command to execute
        modules = ""
        for mod in Flatten(apps[1:]):
            modules += " -m %s " % str(mod.abspath)

        # XXX: why the hell won't this look in the PATH?
        legion_cmd = "tools/legion/legion -k %s %s -o $TARGET" % (Flatten(apps)[0].abspath, modules)

        print "apps: %s" % apps
        print "legion_cmd is '%s'" % legion_cmd
        
        # command object
        boot_image = Command( os.path.join(self.build_dir, "mach_kernel"),
                              apps,
                              Action( legion_cmd , "[LEGN] $TARGET") )

        return boot_image

def env_inner_class(cls):
    """
    Grant a class access to the attributes of a KengeEnvironment
    instance.  This is similar to Java inner classes.

    The weaver objects need access to the machine properties, which
    are stored in KengeEnvironment.machine.  Rather then requiring the
    env to be passed to every constructor,  this function converts
    them into inner classes, so that env.Foo() will create an instance
    of Foo that contains a __machine__ class attribute.
    """
    # Sanity Check
    if hasattr(cls, '__machine__'):
        raise TypeError('Existing attribute "__machine__" '
                        'in inner class')

    class EnvDescriptor(object):
        def __get__(self, env, envclass):
            if env is None:
                raise AttributeError('An enclosing instance for %s '
                                     'is required' % (cls.__name__))
            classdict = cls.__dict__.copy()
            # Add the env's machine attr to the class env.
            classdict['__machine__'] = property(lambda s: env.machine)

            return type(cls.__name__, cls.__bases__, classdict)

    return EnvDescriptor()
    

class KengeEnvironment:
    """The KengeEnvironment creates the environment in which to build
    applications and libraries. he main idea is that everything in an
    environment shares the same toolchain and set of libraries.
    """
    c_ext = ["c"]
    cxx_ext = ["cc", "cpp", "c++"]
    asm_ext = ["s"]
    cpp_asm_ext = ["spp"]
    src_exts = c_ext + cxx_ext + asm_ext + cpp_asm_ext

    def __init__(self, name, SYSTEM, CUST, MACHINE, TOOLCHAIN, BUILD_DIR, **kargs):
        """Create a new environment called name for a given SYSTEM, CUST,
        MACHINE and TOOLCHAIN. It will be built into the specified BUILD_DIR.
        Any extra kargs are saved as extra options to pass on to Packages."""
        
        self.scons_env = Environment() # The underlying SCons environment.
        self.name = name
        self.system = SYSTEM
        self.cust = CUST
        self.machine = MACHINE
        self.builddir = BUILD_DIR
        self.args = kargs
        self.toolchain = TOOLCHAIN
        self.test_libs = []
        self.iguana_servers = []
        self.oklinux_dir = 'linux'
        self.required_vdevs = {}
        self.spawn_vdevs = {}
        self.vdev_counter = {}

        if kargs.has_key('OKLINUX_DIR'):
            self.oklinux_dir = kargs['OKLINUX_DIR']
            del kargs['OKLINUX_DIR']

        def _installString(target, source, env):
            if env.get("VERBOSE_STR", False):
                return "cp %s %s" % (source, target)
            else:
                return "[INST] %s" % target
        self.scons_env["INSTALLSTR"] = _installString

        # Setup toolchain.
        if type(self.toolchain) == type(""):
            raise UserError, "%s is not a valid toolchain, please specify a toolchain found in  tools/toolchains.py" % self.toolchain
        for key in self.toolchain.dict.keys():
            self.scons_env[key] = copy.copy(self.toolchain.dict[key])

        def _libgcc(env):
            """Used by some toolchains to find libgcc."""
            return os.popen(env.subst("$CC $CCFLAGS --print-libgcc-file-name")). \
                   readline()[:-1]
        self.scons_env["_libgcc"] = _libgcc

        def _machine(arg):
            """Return a machine attribute. Used by the toolchains to get
            compiler/machine specific flags."""
            return getattr(self.machine, arg)
        self.scons_env["_machine"] = _machine

        self.headers = []
        self.libs = []

        # Pass through the user's environment variables to the build
        self.scons_env['ENV'] = copy.copy(os.environ)

        # Setup include path
        self.scons_env.Append(CPPPATH = self.builddir  + os.sep + "include")
        self.scons_env["LIBPATH"] = [self.builddir  + os.sep + "lib"]

        # Setup machine defines. Don't really like this. Maybe config.h
        self.scons_env.Append(CPPDEFINES =
                              ["ARCH_%s" % self.machine.arch.upper()])
        if hasattr(self.machine, "arch_version"):
            self.scons_env.Append(CPPDEFINES =
                    [("ARCH_VER", self.machine.arch_version)])
        # FIXME: deal with variant machine names (philipo)
        name = self.machine.__name__.upper()
        if name.endswith('_HW'):
            name = name[:-3]
        self.scons_env.Append(CPPDEFINES = ["MACHINE_%s" % name])
        self.scons_env.Append(CPPDEFINES =
                              ["ENDIAN_%s" % self.machine.endian.upper()])
        self.scons_env.Append(CPPDEFINES =
                              ["WORDSIZE_%s" % self.machine.wordsize])
        self.scons_env.Append(CPPDEFINES = self.machine.cpp_defines)
        self.scons_env.Append(CPPDEFINES = self.machine.cpp_defines)
        if self.machine.smp:
            self.scons_env.Append(CPPDEFINES = ["MACHINE_SMP"])

        self.scons_env.Append(CPPDEFINES =
                              ["KENGE_%s" % self.name.upper()])

        # These should use the Scons SubstitutionEnvironment functions !!
        def _linkaddress(prefix, linkfile):
            addr = eval(open(linkfile.abspath).read().strip())
            return "%s%x" % (prefix, addr)
        self.scons_env["_linkaddress"] = _linkaddress

        def _findlib(inp):
            ret = [self.scons_env.FindFile(
                self.scons_env.subst("${LIBPREFIX}%s${LIBSUFFIX}" % x),
                self.scons_env.subst("$LIBPATH"))
                   for x in inp]
            return ret
        self.scons_env["_findlib"] = _findlib

        def _as_string(inp):
            """Convert a list of things, to a list of strings. Used by
            for example the LINKSCRIPTS markup to convert a list of
            files into a list of strings."""
            return map(str, inp)
        self.scons_env["_as_string"] = _as_string

        def _platform(type, flags):
            """Return the compile flag for the requested compiler"""
            if type == "gnu":
                if flags == "c_flags":
                    return self.machine.c_flags
                # do we want separate cxx_flags?
                if flags == "cxx_flags":
                    return self.machine.c_flags
                if flags == "as_flags":
                    return self.machine.as_flags
                if flags == "link_flags":
                    return self.machine.link_flags
            if type == "rvct":
                if flags == "c_flags":
                    return self.machine.rvct_c_flags
                # do we want separate cxx_flags?
                if flags == "cxx_flags":
                    return self.machine.rvct_c_flags
                if flags == "as_flags":
                    return self.machine.rvct_c_flags
            if type == "ads":
                if flags == "c_flags":
                    return self.machine.ads_c_flags
                # do we want separate cxx_flags?
                if flags == "cxx_flags":
                    return self.machine.ads_c_flags
                if flags == "as_flags":
                    return self.machine.ads_c_flags

            return ""
        self.scons_env["_platform"] = _platform

        ########################################################################
        # Setup and define Magpie tool
        ########################################################################
        def magpie_scanner(node, env, pth):
            node = node.rfile()

            if not node.exists():
                return []

            res = []
            
            if str(node).endswith(".h"):
                CScan.scan(node, pth)
                if node.includes:
                    r = [SCons.Node.FS.find_file(x[1], pth)
                         for x in node.includes]
                    if None in r:
                        #raise UserError, "Couldn't find a header %s when " \
                        print "Couldn't find a header %s when " \
                              "scanning %s" % (node.includes[r.index(None)][1],
                                               node)
                    r = [f for f in r if f]
                    res = Flatten(r)
            else:
                contents = node.get_contents()
                for line in contents.split("\n"):
                    if line.startswith("import"):
                        f = line.split()[-1][1:-2]
                        f = SCons.Node.FS.find_file(f, pth)
                        if f:
                            res.append(f)
            return res

        def magpie_path(env, dir):
            return tuple([Dir(x) for x in env["CPPPATH"]])

        def current_check(node, env):
            c = not node.has_builder() or node.current(env.get_calculator())
            #print "CURRENT CHECK", node, c, node.has_builder()
            return c

        magpie_scan = Scanner(magpie_scanner, recursive=True, skeys=[".idl4"],
                              name="magpie_scanner",
                              path_function = \
                              SCons.Scanner.FindPathDirs("CPPPATH"),
                              scan_check=current_check)
        
        self.scons_env.Append(SCANNERS=magpie_scan)

        self.scons_env["MAGPIE"] = "PYTHONPATH=tools/magpie-parsers/src:tools/magpie " \
                                   "tools/magpie-tools/magpidl4.py"
        self.scons_env["MAGPIE_INTERFACE"] = "v4nicta_n2"
        self.scons_env["MAGPIE_PLATFORM"] = "okl4"        
        builddir = str(Dir(self.builddir))
        magpie_cache_dir = os.path.join(builddir, ".magpie/cache")
        magpie_template_dir = os.path.join(builddir, ".magpie/templates")
        self.scons_env["MAGPIE_BASE_COM"] =  \
          "$MAGPIE --with-cpp=$CPP --cpp-flags \"$_CPP_COM_FLAGS $_CPP_STDIN_EXTRA_FLAGS\" $_CPPINCFLAGS $_CPPDEFFLAGS -h $TARGET -i " + \
          "$MAGPIE_INTERFACE -p $MAGPIE_PLATFORM --magpie-cache-dir=%s --magpie-template-cache-dir=%s --word-size=%d" % (magpie_cache_dir, magpie_template_dir, machine.wordsize)

        self.scons_env["MAGPIE_SERVER_COM"] = "$MAGPIE_BASE_COM -s $SOURCE"
        self.scons_env["MAGPIE_SERVERHEADERS_COM"] = "$MAGPIE_BASE_COM --service-headers $SOURCE"
        self.scons_env["MAGPIE_CLIENT_COM"] = "$MAGPIE_BASE_COM -c $SOURCE"

        self.scons_env["BUILDERS"]["MagpieServerHeader"] = Builder(
            action=Action("$MAGPIE_SERVERHEADERS_COM", "[IDL ] $TARGET"),
            suffix = "_serverdecls.h",
            src_suffix = ".idl4",
            source_scanner = magpie_scan)

        self.scons_env["BUILDERS"]["MagpieServer"] = Builder(
            action=Action("$MAGPIE_SERVER_COM", "[IDL ] $TARGET"),
            suffix = "_serverloop.c",
            src_suffix = ".idl4",
            source_scanner = magpie_scan)

        self.scons_env["BUILDERS"]["MagpieClient"] = Builder(
            action=Action("$MAGPIE_CLIENT_COM", "[IDL ] $TARGET"),
            suffix = "_client.h",
            src_suffix = ".idl4",
            source_scanner = magpie_scan)

        self.scons_env["BUILDERS"]["AsmFile"] = Builder(
            action=Action("$ASMCOM", "$ASMCOMSTR"),
            suffix = ".s",
            source_scanner = SCons.Defaults.CScan,
            )
        self.scons_env["BUILDERS"]["CppFile"] = Builder(
            action=Action("$CPPCOM", "$CPPCOMSTR"),
            source_scanner = SCons.Defaults.CScan
            )

        ########################################################################
        # Setup and define Reg tool
        ########################################################################
        self.scons_env["REG"] = "python tools/dd_dsl.py"
        self.scons_env["REG_COM"] = "$REG -o ${TARGET.dir} $SOURCES"
        self.scons_env["BUILDERS"]["Reg"] = Builder(action = "$REG_COM",
                                                    src_suffix = ".reg",
                                                    suffix = ".reg.h")

        ########################################################################
        # Setup and define dx tool
        ########################################################################
        self.scons_env["DX"] = "python tools/dd_dsl_2.py"
        self.scons_env["DX_COM"] = "$DX $SOURCES $TARGET " + str(MACHINE.arch)
        self.scons_env["BUILDERS"]["Dx"] = Builder(action = "$DX_COM",
                                                    src_suffix = ".dx",
                                                    suffix = ".h")


        ########################################################################
        # Setup and define di tool
        ########################################################################
        self.scons_env["DI"] = "python tools/di_dsl.py"
        self.scons_env["DI_COM"] = "$DI $SOURCES $TARGET"
        self.scons_env["BUILDERS"]["Di"] = Builder(action = Action("$DI_COM", "[DI  ] $TARGET"),
                                                    src_suffix = ".di",
                                                    suffix = ".h")

        ########################################################################
        # Pre-processed builder
        ########################################################################
        self.scons_env["CPPI"] = "python tools/dd_dsl.py"
        self.scons_env["BUILDERS"]["CppI"] = Builder(action = "$CPPCOM",
                                                    suffix = ".i")

        ########################################################################
        # Macro expanding pre-processed builder
        ########################################################################
        self.scons_env["BUILDERS"]["MExp"] = Builder(
            action = Action("$CPPCOM", "[MEXP] $TARGET"),
            suffix = ".mi",
            source_scanner = SCons.Defaults.CScan
            )

        ########################################################################
        # Setup and define markup of templates
        ########################################################################
        def build_from_template(target, source, env, for_signature = None):
            assert len(target) == 1
            assert len(source) == 1
            target = str(target[0])
            source = str(source[0])

            template = file(source).read()
            output = StringIO()
            result = markup(template, output, env["TEMPLATE_ENV"])
            assert result is True
            file(target, 'wb').write(output.getvalue())

        self.scons_env["BUILDERS"]["Template"] = Builder(action = build_from_template,
                                                         src_suffix = ".template",
                                                         suffix = "")
        self.scons_env["TEMPLATE_ENV"] = {}

        ########################################################################
        # Setup and define Flint tool
        ########################################################################
        
        self.scons_env["FLINT"] = "flint"
        self.scons_env["FLINT_OPTIONS"] = ["tools/settings.lnt"]
        self.scons_env["FLINT_OPTIONS"].append("\"+cpp(%s)\"" % ",".join(self.cxx_ext))
        self.scons_env["FLINT_OPTIONS"].append("+sp%d" % (self.machine.wordsize/8))
        # We ensure that long is always the same size as pointer 
        self.scons_env["FLINT_OPTIONS"].append("+sl%d" % (self.machine.wordsize/8))
        def _flint_extra (flint_run, source):
            extras = []
            if flint_run == 1:
                extras.append("-zero")
            return extras

        self.scons_env["_FLINT_EXTRA"] = _flint_extra
        self.scons_env["_FLINT_OPTIONS"] = '${_FLINT_EXTRA(FLINT_RUN)} ${_concat("", FLINT_OPTIONS, "", __env__)}'
        self.scons_env["FLINTCOM"] = "$FLINT $_FLINT_OPTIONS $_FLINT_SUPPRESS $_CPPDEFFLAGS $_CPPINCFLAGS  $SOURCE"
        self.scons_env["FLINT_EXTS"] = self.c_ext + self.cxx_ext

        ########################################################################
        # Setup and define splint tool
        ########################################################################

        self.scons_env["SPLINT"] = "splint"
        self.scons_env["SPLINT_OPTIONS"] = ["-nolib",
                                            "-weak"]
        self.scons_env["_SPLINT_OPTIONS"] = '${_concat("", SPLINT_OPTIONS, "", __env__)}'
        self.scons_env["SPLINTCOM"] = "$SPLINT " + \
                                      "$_SPLINT_OPTIONS " + \
                                      "$_CPPDEFFLAGS " + \
                                      "$_CPPINCFLAGS $SOURCE"
        self.scons_env["SPLINT_EXTS"] = self.c_ext

        #
        # Initialise list of linters
        #
        self.linters = ["FLINT", "SPLINT"]

        for key in kargs:
            self.scons_env[key] = copy.copy(kargs[key])

    def Package(self, package, buildname=None, **kargs):
        """Called to import a package. A package contains source files and
        a SConscript file. It may create multiple Libraries and Applications."""
        # Export env and args to the app SConstruct
        if buildname is None:
            buildname = package.replace(os.sep, "_")
        builddir = os.path.join(self.builddir, "object", buildname)

        args = copy.copy(self.args)
        args.update(kargs)
        if hasattr(conf, self.name) and hasattr(getattr(conf, self.name), buildname):
            kargs.update(getattr(getattr(conf, self.name), buildname).dict)

        env = self
        args["buildname"] = buildname
        args["package"] = package
        args["system"] = self.system
        args["cust"] = self.cust
        args["platform"] = self.machine.platform

        # Call the apps SConstruct file to build it
        paths = [
            os.path.join('cust', self.cust, package),
            package,
            os.path.join(self.oklinux_dir, package),
        ]
        for path in paths:
            sconscript_path = os.path.join(path, 'SConscript')
            if os.path.exists(sconscript_path):
                return SConscript(sconscript_path, build_dir=builddir,
                                  exports = ['env', 'args'], duplicate=0)

        raise UserError("Unable to find package: %s" % package)

       
    def LayoutVirtual(self, apps):
        """Layout a list of objects in memory.  Virtual, physical and
        rommable addresses are calculated separately"""

        def get_next_address(target, source, env):

            def check_fixed_collision(address, size_hint, fixed_apps):
                """Check to see if this address collides with a fixed memsection and
                find the next hole if it does"""

                for app in fixed_apps:
                    if address < app.attributes.weaver.get_last_virtual_addr() and\
                            address > app.attributes.weaver.addressing.virt_addr - size_hint:
                        #find next hole
                        address = align_up(app.attributes.weaver.get_last_virtual_addr(),
                                           app.attributes.weaver.addressing.get_virt_align())
                    elif address <= app.attributes.weaver.addressing.virt_addr - size_hint:
                        break

                return address

            weaver = source[0].attributes.weaver
            alignment = source[1].value
            size_hint = addressing.get_size_hint()
            address = align_up(weaver.get_last_virtual_addr(), alignment)
            address = check_fixed_collision(address, size_hint, fixed_apps)
            open(target[0].abspath, "w").write("0x%xL\n" % address)

        def set_next_address(target, source, env):
            open(target[0].abspath, "w").write("0x%xL\n" % source[0].value)

        apps = Flatten(apps)
        allapps = []
        for app in apps:
            allapps.extend(app.attributes.weaver.subnodes())
            allapps.append(app)

        apps = allapps
        apps = Flatten(apps)

        fixed_apps = []
        non_fixed_apps = []

        for app in apps:
            if app.attributes.weaver.addressing.ignore_virt():
                continue
            elif app.attributes.weaver.addressing.virt_is_fixed():
                fixed_apps.append(app)
            else:
                non_fixed_apps.append(app)

        fixed_apps.sort(key=lambda x: x.attributes.weaver.addressing.virt_addr)

        # hack to remove pistachio after we have made the linkfile
        fixed_apps_to_remove = []

        for app in fixed_apps:
            addressing = app.attributes.weaver.addressing
            linkfile = File(app.abspath + ".linkaddress")
            app.LINKFILE = linkfile

            Depends(app, linkfile)

            # hack to still make linkfile but ignore for future apps
            if addressing.virt_addr == 0x0:
                # L4 on ia32
                fixed_apps_to_remove.append(app)
            else:
                for x in non_fixed_apps:
                    Depends(x, linkfile)

            Command(app.LINKFILE, Value(addressing.virt_addr),
                    Action(set_next_address, "[VIRT] $TARGET"))

        for app in fixed_apps_to_remove:
            fixed_apps.remove(app)

        last_app = None

        for app in non_fixed_apps:
            addressing = app.attributes.weaver.addressing
            linkfile = File(app.abspath + ".linkaddress")
            app.LINKFILE = linkfile

            Depends(app, linkfile)

            if addressing.virt_is_base():
                # rootserver
                Command(app.LINKFILE, Value(addressing.virt_addr),
                        Action(set_next_address, "[VIRT] $TARGET"))
            else:
                # iguana apps
                Command(app.LINKFILE, [last_app,
                                       Value(app.attributes.weaver.get_addressing().get_virt_align())],
                        Action(get_next_address, "[VIRT] $TARGET"))
            last_app = app

        return apps

    def GenWeaverXML(self, apps):
        """Generate a weave XML file."""

        def generate_xml(target, source, env):

            template = ('<?xml version="1.0"?>\n<!DOCTYPE image SYSTEM "weaver-1.1.dtd">\n<image>\n')
            # Add in the machine information
            template += '\t<machine>\n'
            template += '\t\t<word_size size="0x%x" />\n' % self.machine.wordsize

            if len(self.machine.memory['virtual']) > 0:
                template += '\t\t<virtual_memory name="virtual">\n'
                for region in self.machine.memory['virtual']:
                    template += '\t\t\t<region base="0x%x" size="0x%x" />\n' % (region.base, region.size)
                template += '\t\t</virtual_memory>\n'

            for mem in self.machine.memory.keys():
                if mem in ['virtual'] or len(self.machine.memory[mem]) is 0:
                    continue

                template += '\t\t<physical_memory name="%s">\n' % mem
                for region in self.machine.memory[mem]:
                    template += '\t\t\t<region base="0x%x" size="0x%x" type="%s" />\n' % \
                                (region.base, region.size, region.mem_type)
                template += '\t\t</physical_memory>\n'

            if hasattr(self.machine, 'v2_drivers'):
              for (driver, vdev, mems, irqs) in self.machine.v2_drivers:
                  template += '\t\t<phys_device name="%s_dev">\n' % vdev[1:] # strip the first char which should be 'v'

                  for i, (base, end) in enumerate(mems):
                      template += '\t\t\t<physical_memory name="%s_mem%d">\n' % (vdev[1:], i)
                      template += '\t\t\t\t<region base="0x%x" size="0x%x" />\n' % (base, end - base)
                      template += '\t\t\t</physical_memory>\n'

                  for i, irq in enumerate(irqs):
                      template += '\t\t\t<interrupt name="int_%s%d" number="%d" />\n' % (vdev[1:], i, irq)

                  template += '\t\t</phys_device>\n'

            for size in self.machine.page_sizes:
                template += '\t\t<page_size size="0x%x" />\n' % size
            template += '\t</machine>\n'

            for app in source:
                template += '\t' + app.attributes.weaver.element(env = self) + '\n'

            template += "</image>\n"

            f = open(target[0].abspath, "w")
            f.write(template)
            f.close()

        weaver = os.path.join(self.builddir, "weaver.xml")
        apps = Flatten(apps)
        spec = Command(weaver,
                       apps,
                       Action(generate_xml, "[XML ] $TARGET"))

        return spec

    def CreateImages(self, weaverxml, apps):
        elf_image = Command(os.path.join(self.builddir, "image.elf"),
                            weaverxml,
                            Action("tools/pyelf/elfweaver --traceback merge " + \
                                   "-o$TARGET $SOURCE", "[ELF ] $TARGET"))

        if self.machine.zero_bss == True:
            elf_image = Command(os.path.join(self.builddir, "image.elf.nobits"),
                                elf_image,
                                Action("tools/pyelf/elfweaver modify " + \
                                       "-o$TARGET $SOURCE --remove_nobits", "[ELF ] $TARGET"))

        if self.machine.copy_elf == True:
            elf_image = Command(os.path.join(self.builddir, "../image.elf"),
                    elf_image,
                    Action("cp $SOURCE $TARGET", "[ELF ] $TARGET"))

        Depends(elf_image, apps)

        boot_image = Command(os.path.join(self.builddir, "image.boot"),
                            elf_image,
                            Action("tools/pyelf/elfweaver modify " + \
                                   "-o$TARGET $SOURCE --physical_entry --physical", "[ELF ] $TARGET"))

        if self.machine.arch == "mips":
            boot_image = Command(os.path.join(self.builddir, "image.mips"),
                                elf_image,
                                Action("tools/pyelf/elfweaver modify " + \
                                       "-o$TARGET $SOURCE --physical_entry --physical --adjust segment.paddr +0x80000000", "[ELF ] $TARGET"))

        if self.machine.boot_binary == True:
            boot_image = self.scons_env.Command(os.path.join(self.builddir, "image.boot.bin"),
                                 boot_image,
                                 Action("$OBJCOPY -O binary $SOURCE $TARGET", "[BIN ] $TARGET"))

        Depends(boot_image, apps)

        sim_image = Command(os.path.join(self.builddir, "image.sim"),
                           elf_image,
                           Action("tools/pyelf/elfweaver modify " + \
                                  "-o$TARGET $SOURCE --physical_entry", "[ELF ] $TARGET"))

        if self.machine.arch == "mips":
            sim_image = Command(os.path.join(self.builddir, "image.sim"),
                                elf_image,
                                Action("tools/pyelf/elfweaver modify " + \
                                       "-o$TARGET $SOURCE --physical_entry --physical --adjust segment.paddr +0x80000000", "[ELF ] $TARGET"))
 
        Depends(sim_image, apps)

        return [elf_image, sim_image, boot_image]

    def CreateFatImage(self, install_bootable):
    
        # Generate an image that can be used for the simulator...
        sim_menulst = Command("%s/grub/menu.lst" % self.builddir, elf_image,
                              self.BuildMenuLst, ROOT="/boot/grub")
                
        install_bootable += sim_menulst
        sim_image = Command("%s/c.img" % self.builddir, install_bootable , self.GrubBootImage)

        return sim_image
        
    def BuildMenuLst(self, target, source, env):
        menulist_serial = """
        serial --unit=0 --stop=1 --speed=115200 --parity=no --word=8
        terminal --timeout=0 serial console
        """

        menulst_data = """
        default 0
        timeout = 0
        title = L4 NICTA::Iguana
        root (hd0,0)
        kernel=%s/image.elf
        """
        menulst = target[0]
        dir = env["ROOT"]

        menu = menulist_serial + menulst_data % dir 

        open(str(menulst), "w").write(menu)

    def GrubBootImage(self, target, source, env):
        GRUB_BIN = "tools/grub/grub"
        GRUB_STAGE_1 = "tools/grub/stage1"
        GRUB_STAGE_2 = "tools/grub/stage2"
        GRUB_STAGE_FAT = "tools/grub/fat_stage1_5"

        output_image = target[0]

        os.system("strip %s" % elf_image[0])

        bmap = "%s/grub/bmap" % self.builddir[1:]
        print bmap
        mtoolsrc = "%s/grub/mtoolsrc" % self.builddir[1:]
        print mtoolsrc

        open(bmap, "w").write("(hd0) %s" % output_image)
        print "wrote bmap"
        open(mtoolsrc, "w").write('drive c: file="%s" partition=1\n' % output_image)
        print "wrote mtoolsrc"
        # This needs to by dynamically set (from disk_size in projects/iguana/SConstruct?)
        size = 48000
        sectors = size * 1024 / 512
        tracks = sectors / 64 / 16

        def mtoolcmd(str):
            #return os.system("MTOOLSRC=%s tools/%s" % (mtoolsrc, str))
            error =  os.system("MTOOLSRC=%s tools/mtools/%s" % (mtoolsrc, str))
            if error != 0:
                raise UserError, "%s returned error, perhaps you haven't got mtools-bin installed?" % str
            else:
                return 0

        def copy(file):
            # Hack! If file ends in .reloc, strip it!
            if file.abspath.endswith(".reloc"):
                filenm = file.abspath[:-6]
            elif file.abspath.endswith(".gz"):
                filenm = file.abspath[:-3]
            else:
                filenm = file.abspath


            x = mtoolcmd("mcopy -o %s c:/boot/grub/%s" % (file.abspath, os.path.basename(filenm)))
            if x != 0:
                os.system("rm %s" % output_image)
                raise SCons.Errors.UserError, "Bootimage disk full"

        os.system("dd if=/dev/zero of=%s count=%s bs=512" % (output_image, sectors) ) # Create image
        mtoolcmd("mpartition -I c:")
        mtoolcmd("mpartition -c -t %s -h 16 -s 63 c:" % tracks) #
        mtoolcmd("mformat c:") # Format c:
        mtoolcmd("mmd c:/boot")
        mtoolcmd("mmd c:/boot/grub")
        mtoolcmd("mcopy %s c:/boot/grub" % GRUB_STAGE_1)
        mtoolcmd("mcopy %s c:/boot/grub" % GRUB_STAGE_2)
        mtoolcmd("mcopy %s c:/boot/grub" % GRUB_STAGE_FAT)

        os.system("""echo "geometry (hd0) %s 16 63
                  root (hd0,0)
                  setup (hd0)" | %s --device-map=%s --batch
                  """ % (tracks, 'tools/grub/grub', bmap))

        for each in source:
            print "copying %s to bootdisk" % each.abspath
            copy(each)

    ############################################################################
    # Methods called by SConscript
    ############################################################################
    class _extra:
        def __init__(self, data):
            self.data = data

    def _mogrify(self, kargs):
        for key in kargs:
            if isinstance(kargs[key], self._extra):
                kargs[key] = self.scons_env[key] + kargs[key].data
        return kargs

    def Extra(self, arg):
        return self._extra(arg)

    def Append(self, *args, **kargs):
        return self.scons_env.Append(*args, **self._mogrify(kargs))

    def AsmFile(self, *args, **kargs):
        return self.scons_env.AsmFile(*args, **self._mogrify(kargs))

    def CppFile(self, *args, **kargs):
        return self.scons_env.CppFile(*args, **self._mogrify(kargs))

    def CopyFile(self, target, source):
        return self.scons_env.InstallAs(target, source)

    def Command(self, target, source, action, **kargs):
        return self.scons_env.Command(target, source, action, **self._mogrify(kargs))

    def Depends(self, target, source):
        return self.scons_env.Depends(target, source)

    def weaver(self, target):
        if isinstance(target, SCons.Node.NodeList):
            target = target[0]

        return target.attributes.weaver

    def set_weaver(self, target, weaver, xml_name):
        if isinstance(target, SCons.Node.NodeList):
            target = target[0]

        target.attributes.weaver = weaver
        weaver.set_node(target, xml_name)

    def CopyTree(self, target, source):
        ret = []
        for root, dirs, files in os.walk(Dir(source).srcnode().abspath, topdown=True):
            if "=id" in files:
                files.remove("=id")
            if ".arch-ids" in dirs:
                dirs.remove(".arch-ids")
            stripped_root = root[len(Dir(source).srcnode().abspath):]
            dest = Dir(target).abspath + stripped_root
            if files:
                for f in files:
                    src = root + os.sep + f
                    des = dest + os.sep + f
                    ret += self.InstallAs(des, src)
            elif not dirs:
                ret += self.Command(Dir(dest), [], Mkdir(dest))
        return ret

    def Ext2FS(self, size, dev, name=None, cache_policy = None):
        genext2fs = SConscript(os.path.join(self.oklinux_dir, "tools", "genext2fs", "SConstruct"),
                               build_dir=os.path.join(self.builddir, "native", "tools", "genext2fs"),
                               duplicate=0)
        ramdisk = self.builddir + os.sep + "ext2ramdisk"
        cmd = self.Command(ramdisk, Dir(os.path.join(self.builddir,
                                                     "install")),
                           genext2fs[0].abspath +
                           " -b $EXT2FS_SIZE -d $SOURCE -f $EXT2FS_DEV $TARGET",
                           EXT2FS_SIZE=size, EXT2FS_DEV=dev)
        self.set_weaver(cmd, self.WeaverFile(cache_policy = cache_policy), name)
        Depends(cmd, genext2fs)

        # always regenerate the ramdisk file
        AlwaysBuild(cmd)
        return cmd

    def _find_source(self, buildname, test_lib=False):
        """This function returns the globs used to automagically find
        source when the user doesn't supply it explicitly."""
        source = []
        if buildname == "ig_server":
            buildname = os.path.join("iguana","server") 
        dirs = ["src/",
                "#arch/%s/%s/src/" % (self.machine.arch, buildname),
                "#arch/%s/%s/src/toolchain-%s/" % (self.machine.arch, buildname, self.toolchain.name),
                "#arch/%s/libs/%s/src/" % (self.machine.arch, buildname), 
                "#cust/%s/arch/%s/%s/src/" % (self.cust, self.machine.arch, buildname),
                "#cust/%s/arch/%s/%s/src/toolchain-%s/" % (self.cust, self.machine.arch, buildname, self.toolchain.name),
                "#cust/%s/%s/cust/src/" % (self.cust, buildname),
                "#cust/%s/%s/cust/src/toolchain-%s/" % (self.cust, buildname, self.toolchain.name),
                "system/%s/src/" % self.system
                ]
        if test_lib:
            dirs.append("test/")
        for src_ext in self.src_exts:
            for dir_ in dirs:
                source.append(dir_ + "*." + src_ext)
        return source

    
    def _setup_linters(self, objects):
        """Setup any linters to run on the list of objects."""
        for linter in self.linters:
            if "%s_RUN" % linter in self.args:
                for obj in objects:
                    ext = str(obj.sources[0]).split(".")[-1]
                    if ext in self.scons_env["%s_EXTS" % linter]:
                        self.scons_env.AddPreAction(obj, Action("$%sCOM" % linter, "[LINT] $SOURCE"))

    def KengeIDL(self, source=None):
        """
        Build IDL files into client and server header and source
        files.  The function returns a tuple containing:

        1) A list of nodes of client headers.  This can be passed to
           KengeLibrary() via the extra_headers keyword.
        2) A list of nodes of client source files.  This can be passed to
           KengeLibrary() via the extra_source keyword.
        3) A hash of server header files.  The key is the basename of
           the IDL file.
        4) A hash of server source files.  The key is the basename of
           the IDL file.  The hash value can be passed to KengeProgram
           via the extra_source keyword.

        The use of hashes in (3) and (4) is because the source list to
        this function can contain multiple IDL files, but a server may
        only implement one of the interfaces.

        The implementation currently uses the Magpie IDL compiler.
        """
        if source is None:
            source = ["include/interfaces/*.idl4"]

        idl4_files = Flatten([src_glob(glob) for glob in source])

        server_headers = {}
        server_src     = {}
        client_headers = []
        client_src     = []

        for src in idl4_files:
            srv  = self.scons_env.MagpieServer(src)[0]
            dest = os.path.join(self.builddir, str(srv))
            server_src[os.path.basename(src)] = self.scons_env.InstallAs(dest, str(srv))

            srv_header = self.scons_env.MagpieServerHeader(src)[0]
            self.scons_env.Depends(srv_header, srv)
            dest = os.path.join(self.builddir, str(srv_header))
            client_headers += self.scons_env.InstallAs(dest, str(srv_header))

            cli  = self.scons_env.MagpieClient(src)[0]
            dest = os.path.join(self.builddir, str(cli))
            client_headers += self.scons_env.InstallAs(dest, str(cli))

        # Magpie does not generate client src files, but include the
        # possibility in the API in case the implementation changes.
        return (client_headers, client_src, server_headers, server_src)

    def add_public_headers(self, name, buildname, public_headers):
        if public_headers is None:
            public_headers = [("include/", "include/%(name)s/"),
                              ("#arch/%(arch)s/%(name)s/include/", "include/%(name)s/arch/"),
                              ("#arch/%(arch)s/libs/%(name)s/include/", "include/%(name)s/arch/"), 
                              ("#cust/%(cust)s/%(name)s/include/", "include/%(name)s/cust/"),
                              ("#cust/%(cust)s/arch/%(arch)s/libs/%(name)s/include/", "include/%(name)s/arch/"),
                              ("system/%(system)s/include/", "include/%(name)s/system/"),
                              ("toolchain/%(toolchain)s/include/", "include/%(name)s/toolchain/")
                              ]
            if self.test_libs and name in self.test_libs:
                public_headers.append(("test/", "include/%(name)s/"))

        # Make sure the Magpie headers are copied into the build directory
        public_headers.append(("#tools/magpie/include/idl4biguuid/", "include/idl4biguuid/"))
        public_headers.append(("#tools/magpie/include/idl4biguuid/api/", "include/idl4biguuid/api/"))
        public_headers.append(("#tools/magpie/include/idl4biguuid/api/v4/", "include/idl4biguuid/api/v4/"))
        public_headers.append(("#tools/magpie/include/idl4biguuid/arch/", "include/idl4biguuid/arch/"))
        public_headers.append(("#tools/magpie/include/idl4biguuid/arch/generic_biguuid/", "include/idl4biguuid/arch/generic_biguuid/"))
        public_headers.append(("#tools/magpie/include/idl4biguuid/glue/", "include/idl4biguuid/glue/"))
        public_headers.append(("#tools/magpie/include/idl4biguuid/glue/v4-generic_biguuid/", "include/idl4biguuid/glue/v4-generic_biguuid/"))

        src_replacements = {"arch" : self.machine.arch,
                            "cust" : self.cust,
                            "system" : self.system,
                            "toolchain" : self.toolchain.name,
                            "name" : buildname,
                            }
        dst_replacements = {"name" : buildname}

        for src, dest in public_headers:
            headers = src_glob( (src + "*.h") % src_replacements)
            for header in headers:
                dir, header_file = os.path.split(header)
                destfile = os.path.join(self.builddir, dest % dst_replacements)
                destfile = os.path.join(destfile, header_file)
                self.headers += self.scons_env.InstallAs(destfile, header)

    def strip_suffix(self, filename):
        pos = filename.rfind(".")
        if pos == -1:
            return filename
        return filename[:pos]

    def source_to_obj(self, filename, suffix):
        base = self.strip_suffix(filename)
        if base.startswith('#'):
            base = base[1:]
        return base


    def KengeLibrary(self, name, buildname=None, source=None, extra_source=None,
                     public_headers=None, extra_headers=None, dx_files=None,
                     di_files=None, reg_files=None, **kargs):
        """
        source is a list of 'globs' describing which files to compile.
        """
        kargs = self._mogrify(kargs)
        library_args = kargs

        # We want to make sure the source directory is in the include path
        # for private headers
        if "CPPPATH" not in library_args:
            library_args["CPPPATH"] = copy.copy(self.scons_env["CPPPATH"])
        library_args["CPPPATH"].append(Dir("src").abspath)

        if buildname is None:
            buildname = name

        # Handle .di files
        if di_files is None:
            di_files = ["include/*.di"]

        di_file_list = Flatten([src_glob(glob) for glob in di_files])
        for fl in di_file_list:
            di_header = self.scons_env.Di(fl)
            self.scons_env.Install(self.builddir + "/include/driver/", di_header)


        # First step in building a library is installing all the
        # header files.
        self.add_public_headers(name, buildname, public_headers)

        if extra_headers is not None:
            self.headers += extra_headers

        if reg_files:
            reg_file_list = Flatten([src_glob(glob) for glob in reg_files])
            for fl in reg_file_list:
                reg_header = self.scons_env.Reg(fl)

        # Handle .dx files
        if dx_files is None:
            dx_files = ["src/*.dx"]

        dx_file_list = Flatten([src_glob(glob) for glob in dx_files])

        for fl in dx_file_list:
            dx_header = self.scons_env.Dx(fl)


        if source is None:
            test_lib = self.test_libs and name in self.test_libs
            source = self._find_source(name, test_lib=test_lib)

        if extra_source is not None:
            source += extra_source

        source_list = Flatten([src_glob(glob) for glob in source])

        obj_dir = os.path.join(self.builddir, "object", "libs_" + name)
        objects = [self.scons_env.StaticObject(os.path.join(obj_dir, self.source_to_obj(src, '.o')), src, **library_args) for src in source_list]
        objects = Flatten(objects)

        self._setup_linters(objects)

        lib_name = self.builddir +  os.sep +  "lib"  + os.sep + "lib%s.a" % buildname
        lib = self.scons_env.StaticLibrary(lib_name, objects, **library_args)

        self.libs += lib
        
        return lib

    def KengeProgram(self, name, buildname=None, source=None, extra_source=None,
                     public_headers=None, weaver=None, **kargs):
        """
        source is a list of 'globs' describing which files to compile.
        """
        kargs = self._mogrify(kargs)
        program_args = kargs
        program_args["CPPPATH"] = program_args.get("CPPPATH",
                                                   copy.copy(self.scons_env["CPPPATH"]))
        program_args["CPPPATH"].append(Dir("include"))

        depend_prefix = ""
        if "LINKSCRIPTS" not in program_args:
            # Is a default linker script provided?
            if (hasattr(self.machine, "link_script")):
                depend_prefix = "#"     # file comes from source dir, not builddir
                program_args["LINKSCRIPTS"] = self.machine.link_script

        if program_args.has_key('LIBS'):
            libs = sets.Set(program_args['LIBS'])
            program_args['LIBS'] = list(libs)
        else:
            program_args['LIBS'] = []

        if buildname is None:
            buildname = name

        self.add_public_headers(name, buildname, public_headers)

        if source is None:
            source = self._find_source(name)

        source_list = Flatten([src_glob(glob) for glob in source])

        # EVIL...
        template_files = Flatten(src_glob("src/*.template"))

        for file_name in template_files:
            # FIXME this is hacky
            template_env = kargs.get("TEMPLATE_ENV", self.scons_env["TEMPLATE_ENV"])
            template = self.scons_env.Template(file_name, TEMPLATE_ENV=template_env)
            self.scons_env.Depends(template, Value(template_env))
            source_list.append(str(template[0]))

        obj_dir = os.path.join(self.builddir, name, "object") 
        objects = [self.scons_env.StaticObject(os.path.join(obj_dir, self.source_to_obj(src, '.o')), src, **program_args) for src in source_list if not src.endswith('.template')]
        objects = Flatten(objects)

        # Support for a list of pre-processed objects
        # eg. as needed by x86_64 for init32.cc
        if extra_source is not None:
            objects += extra_source

        # CEG hack
        if name == "l4kernel" and (self.machine.arch == "x86_64" or
                                   self.machine.arch == "ia32"):
            for obj in objects:
                self.Depends(obj, "include/tcb_layout.h")
                self.Depends(obj, "include/ktcb_layout.h")

        # Generate targets for .i, pre-processed but not compiled files.
        # these aren't depended on by anything and target must be explicitly
        # defined on the command line to get them built.
        [self.scons_env.CppI(x, **program_args)
         for x in source_list if not x.endswith(".template")]

        self._setup_linters(objects)

        # For GNU compilers, include our libgcc if needed
        if self.toolchain.type == "gnu":
            if self.toolchain.dict["LIBGCC"] == "-lgcc":
                program_args['LIBS'] += ["gcc"]

        output = self.builddir + os.sep + "bin" + os.sep + buildname
        prog = self.scons_env.Program(output, objects, **program_args)

        # By default programs are plain elf files.
        if weaver is None:
            weaver = self.WeaverElfFile()

        self.set_weaver(prog, weaver, name)

        # FIXME: This should be part of the program scanner, libc might
        # be built after program
        if self.scons_env.get("CRTOBJECT"):
            self.Depends(prog, self.scons_env["CRTOBJECT"])

        if "LINKSCRIPTS" in program_args and program_args["LINKSCRIPTS"] is not None: 
            self.Depends(prog, depend_prefix + program_args["LINKSCRIPTS"])

        return prog

    def KengeCRTObject(self, source, **kargs):
        # Rule to build a C runtime object.
        output = self.builddir + os.sep + "lib" + os.sep + "crt0.o"
        source_list = Flatten([src_glob(glob) for glob in source])
        if source_list == []:
            raise UserError, "No valid object in " + \
                  "KengeCRTObject from glob: %s" % source
        obj = self.scons_env.StaticObject(output, source_list, **kargs)
        self.scons_env["CRTOBJECT"] = obj

    def IguanaMemoryPool(self, name, memory):
        """
        Declare a memory pool for iguana.
        """
        output = os.path.join(self.builddir, "bin", name + "_memory_pool")
        pool = Command(output,
                       [], 
                       Touch('$TARGET'))

        self.set_weaver(pool,
                        self.WeaverIguanaMemoryPool(name, memory),
                        name)

        return pool

    def IguanaDefaultMemoryPools(self, iguana_server):
        """
        Convert the memory ranges specified in machines.py into iguana
        memory pools.

        Iterate through the memory types for the current machine (as
        found in self.machines.memory) and process them in the
        following ways:

        1) If the memory type is in the skipped_memory list then skip

        2) If the memory is called 'virtual' create a virtual pool

        3) For other memory types, create a physical memory pool

        The root program default pools are set to be the pools called
        'virtual' and 'physical'.
        """
        pools = []
        skipped_memory = ()

        for mem in self.machine.memory.keys():
            if mem in skipped_memory or len(self.machine.memory[mem]) is 0:
                continue

            output = os.path.join(self.builddir, "bin", mem + "_memory_pool")
            pool = Command(output,
                           [], 
                           Touch('$TARGET'))

            if mem == 'virtual':
                weaver = self.WeaverVirtualMemoryPool(src = mem)
                self.set_weaver(pool, weaver, mem)
            else:
                weaver = self.WeaverPhysicalMemoryPool(src = mem)
                self.set_weaver(pool, weaver, mem)

            pools.append(pool)

        self.weaver(iguana_server).set_default_pools('virtual', 'physical')
        return pools


    def IguanaMemSection(self, name, base, size, flags = 0,
                         cache_policy = None, zero = True, addressing=None):
        """
        Declare a iguana memsection.

        This memsection is created at boottime by server and
        does not take up space in the ELF image.

        name     - The name of memsection in the XML file.
        base     - The base address of the memsection.  The values
                   are:
                   KengeEnvironment.ADDR_ALLOC_RUNTIME - Location chosen at
                   runtime from the virtual memory pool.
                   KengeEnvironment.ADDR_ALLOC - The memsection
                    is layed out in virtual memory along with the
                    other sections on the elf image.  This is useful
                    when a memsection needs to be placed within a
                    certain range of an application
                   Integer - The virtual address of the memsection
        size     - The size in bytes.
        flags    - Memsection memory type flags (MEM_NORMAL, MEM_FIXED, etc). (not used)
        cache policy - L4 memory attributes ("default", "uncached",
                       etc). None means default memory.
        zero     - Zero the memsection on creation (defaults to True).
        addressing - Optional memory addressing options for fine
                     tuning the memory layout.
        """
        ms = Command(os.path.join(self.builddir, "bin", "memsection_" + str(name)),
                     [],
                     Touch('$TARGET'))
        
        self.set_weaver(ms,
                        self.WeaverMemSection(base, size,
                                              flags, cache_policy,
                                              zero,
                                              addressing = addressing),
                        name)

        return ms


    ############################################################################
    # Weaver attribute types and access functions.
    #
    # Weaver attributes describe the properties of an image target
    # that are needed by elfweaver.
    #
    # An image target is a SCons node that is to be copied into the
    # bootimage
    ############################################################################

    class _WeaverAddressing(object):
        """
        Memory allocation properties for an ELF section.

        This class describes how an ELF section should be laid out in
        virtual and physical memory.  The properties for each can be
        specified separately.  If values for these properties are not
        supplied in the constructor then reasonable defaults are 
        given.

        There are three properties that can be described:

        1) XIP

        If a segment is read and/or execute only then it should be
        placed in ROM.

        Allowable values are True and False.

        2) Address

        The address if the segment in virtual or physical memory.
        Allowable values are:

        ADDR_IGNORE - The segment has no meaningful address and should
        be ignored.

        ADDR_BASE - The segment should be placed at the bottom of the
        address range.

        ADDR_AS_BUILT - The address as it appears in the ELF segment
        is the address of the segment.  AKA trust the compiler

        ADDR_ALLOC - The address should be allocate by the build
        system.

        ADDR_ALLOC_RUNTIME - The address should be allocated by Iguana
        at runtime.

        <integer> - The absolute address of the segment.

        3) Alignment

        The alignment of the segment in memory.  Allowable values are:

        ALIGN_ELF - Use the alignment as specified in the ELF segment
        header.  AKA trust the compiler.

        ALIGN_MIN - Use the minimum page size for the target
        architecture.

        ALIGN_PREFERRED - The best static alignment for this
        architecture.  This is a single value that is a good trade off
        between TLB usage and memory wastage.

        ALIGN_SUPERPAGE - Align to the largest page size that is
        smaller that the size of the segment.  Due to the limitations
        in the build system, this value can only be used for physical
        addresses.

        4) Size Hint

        If the program is expected to take more than 1MB of memory,
        set the size hint to be an overestimate of the size, so that
        it does not collide with any fixed memsections.

        5) Pools

        Specify the regions of memory from which segments are allocated.
        """
        ALIGN_MIN       = -1
        ALIGN_PREFERRED = -2
        ALIGN_SUPERPAGE = -3
        ALIGN_ELF       = -4
        ADDR_BASE       = -5
        ADDR_ALLOC      = -6
        ADDR_AS_BUILT   = -7
        ADDR_IGNORE     = -8
        ADDR_ALLOC_RUNTIME = -9

        def __init__(self, machine,
                     xip = False,
                     virt_align = ALIGN_PREFERRED,
                     phys_align = ALIGN_SUPERPAGE,
                     virt_addr  = ADDR_ALLOC,
                     phys_addr  = ADDR_ALLOC,
                     virtpool   = None,
                     physpool   = None,
                     pager      = None,
                     direct     = None,
                     size_hint  = 0x100000):


            # Convert MIN and PREFERRED alignments to their machine
            # specific values.
            if virt_align == KengeEnvironment.ALIGN_MIN:
                virt_align = machine.min_alignment

            if phys_align == KengeEnvironment.ALIGN_MIN:
                phys_align = machine.min_alignment

            if virt_align == KengeEnvironment.ALIGN_PREFERRED:
                virt_align = machine.preferred_alignment
                self.default_align = True
            else:
                self.default_align = False

            if phys_align == KengeEnvironment.ALIGN_PREFERRED:
                phys_align = machine.preferred_alignment

            if virt_addr == KengeEnvironment.ADDR_BASE:
                virt_addr = machine.base_vaddr
                self.virt_base = True
            else:
                self.virt_base = False

            self.xip        = xip
            self.virt_align = virt_align
            self.phys_align = phys_align
            self.virt_addr  = virt_addr
            self.phys_addr  = phys_addr
            self.virtpool   = virtpool
            self.physpool   = physpool
            self.pager      = pager
            self.direct     = direct
            self.size_hint  = size_hint
            self.decreasing_page_sizes = machine.page_sizes[:]
            self.decreasing_page_sizes.sort()
            self.decreasing_page_sizes.reverse()
            self.zone_end   = False

        def virt_is_fixed(self):
            """Is the virtual address at a fixed location?"""
            return self.virt_addr != self.__class__.ADDR_ALLOC and not self.virt_base

        def phys_is_fixed(self):
            """Is the physical address at a fixed location?"""
            return self.phys_addr != self.__class__.ADDR_ALLOC

        def use_phys_as_built(self):
            """Use the physical address as found in the ELF file?"""
            return self.phys_addr == self.__class__.ADDR_AS_BUILT

        def ignore_virt(self):
            """Should this virtual address be ignored?"""
            return (self.virt_addr == self.__class__.ADDR_IGNORE or
                    self.virt_addr == self.__class__.ADDR_ALLOC_RUNTIME)

        def ignore_phys(self):
            """Should this physical address be ignored?"""
            return (self.phys_addr == self.__class__.ADDR_IGNORE or
                    self.phys_addr == self.__class__.ADDR_ALLOC_RUNTIME)

        def virt_is_base(self):
            """Place segment at the bottom on virtual memory?"""
            return self.virt_base

        def do_xip(self):
            """Support Execute In Place?"""
            return self.xip

        def get_virtpool(self):
            """Return the virtual address pool."""
            return self.virtpool

        def get_physpool(self):
            """Return the physical address pool."""
            return self.physpool

        def get_pager(self):
            """Return the pager."""
            return self.pager

        def get_direct(self):
            """Return the direct flag."""
            return self.direct

        def set_zone_end(self, end):
            """
            Indicate whether or not the weaver item is at the end of a
            zone.
            """
            self.zone_end = end

        def align_zone(self, addr):
            """
            Move an address to the end of the zone it is it the last item.

            This is bit hacky.  When placing items in a zone, no other
            items be placed between the last item and end of the
            zone.  If this is the last item, then change address
            returned to the end of the zone so that LayoutVirtual will
            not put the next item in the same 1M region.
            """
            if self.zone_end:
                return align_up(addr, 1024 * 1024) - 1
            else:
                return addr

        def has_default_alignment(self):
            return self.default_align

        def get_virt_align(self):
            """Return the alignment in virtual memory"""
            return self.virt_align

        def get_phys_align(self, size, elf_align):
            """
            Return the alignment in physical memory.

            size      - Size of the segment in bytes.
            elf_align - The alignment found in the ELF file.
            """
            if self.phys_align == self.__class__.ALIGN_ELF:
                return elf_align
            elif self.phys_align == self.__class__.ALIGN_SUPERPAGE:
                # Calc superpage alignment.
                alignment = -1
            
                for i in self.decreasing_page_sizes:
                    if i < size:
                        alignment = i
                        break

                if alignment < 0:
                    alignment = self.decreasing_page_sizes[-1]

                return alignment
            else:
                return self.phys_align

        def get_phys_addr(self):
            """Return the physical address of the segment."""
            return self.phys_addr

        def get_size_hint(self):
            """Return the size hint or 0x100000 if it is smaller."""
            return max(self.size_hint, 0x100000)

    class WeaverELFSegment(object):
        """
        Description of an build ELF segment
        """
        def __init__(self, size = -1, virt_addr = -1,
                     rommable = False, elfsegment = None,
                     phys_from_elf = False, physpool = None):
            self.physpool = physpool
            self.attach   = None
            self.pager    = None

            if elfsegment is not None:
                self.size = elfsegment.get_memsz()
                self.rommable = (elfsegment.flags & PF_W) == 0
                self.virt_addr = elfsegment.vaddr
                self.seg_align = elfsegment.align
                self.flags = elfsegment.flags
                if phys_from_elf:
                    self.phys_addr = elfsegment.paddr
                else:
                    self.phys_addr  = None
            else:
                self.size = size
                self.virt_addr = virt_addr
                self.phys_addr  = None
                self.rommable = rommable
                self.seg_align = -1
                self.flags = 0
            

        def get_size(self):
            """Return the size of the segment in bytes."""
            return self.size

        def get_phys_addr(self):
            """Return the physical address of the segment."""
            return self.phys_addr

        def get_virt_addr(self):
            """Return the virtual address of the segment."""
            return self.virt_addr

        def get_physpool(self):
            """Return the physical pool of the segment."""
            return self.physpool

        def get_attach(self):
            """Return the attach permissions of the segment."""
            return self.attach

        def get_pager(self):
            """Return the pager of the segment."""
            return self.pager

        def get_flags(self):
            """Return the flags of the segment."""
            return self.flags

        def is_rommable(self):
            """Can the segment be placed in ROM?"""
            return self.rommable

        def get_align(self, addressing):
            """Return the desired alignment for this segment."""
            return addressing.get_phys_align(self.size, self.seg_align)

    class WeaverSegment(object):
        """Description of an ELF segment."""

        def __init__(self, name, phys_addr = None,
                     physpool = None, align = None,
                     attach = None, pager = None, direct = None,
                     protected = None, cache_policy = None):
            self.name      = name
            self.phys_addr = phys_addr
            self.physpool  = physpool
            self.align     = align
            self.attach    = attach
            self.pager     = pager
            self.direct    = direct
            self.protected = protected
            self.cache_policy = cache_policy

        def element(self, env = None):
            template = '\t\t<segment name="%s" ' % self.name

            if self.phys_addr is not None:
                template += 'phys_addr="0x%x" ' % self.phys_addr

            if self.physpool is not None:
                template += 'physpool="%s" ' % self.physpool

            if self.align is not None:
                template += 'align="0x%x" ' % self.align

            if self.attach is not None:
                template += 'attach="%s" ' % self.attach

            if self.pager is not None:
                template += 'pager="%s" ' % self.pager

            if self.direct is not None:
                template += 'direct="0x%x" ' % self.direct

            if self.protected is not None:
                template += 'protected="%s" ' % str(self.protected).lower()

            if self.cache_policy is not None:
                template += 'cache_policy="%s" ' % self.cache_policy

            template  += ' />\n'
            return template

    class WeaverZone(object):
        """Description of a Zone."""

        def __init__(self, name, virtpool = None, contents=None):
            self.name = name
            self.virtpool = virtpool
            self.contents = []

            if contents is not None:
                for c in contents:
                    if isinstance(c, SCons.Node.NodeList):
                        self.contents.append(c[0])
                    else:
                        self.contents.append(c)

            real = [c for c in self.contents
                    if isinstance(c, SCons.Node.Node)]

            if len(real) > 0:
                real[-1].attributes.weaver.addressing.set_zone_end(True)

        def subnodes(self):
            return [c for c in self.contents if isinstance(c, SCons.Node.Node)]

        def element(self):
            template = '\t\t<zone name="%s" ' % self.name

            if self.virtpool is not None:
                template += 'virtpool="%s" ' % self.virtpool

            template += '>\n'

            for c in self.contents:
                if isinstance(c, KengeEnvironment.WeaverSegment):
                    template += '\t' + c.element()
                else:
                    template += '\t\t\t' + c.attributes.weaver.element() + '\n'

            template += '\t\t</zone>\n'

            return template

    class WeaverPatch(object):
        """Description of a segment patch."""

        def __init__(self, address, value, bytes = None):
            self.address = address
            self.value   = value
            self.bytes   = bytes

    class _WeaverThread(object):
        """Description of extra threads to start for a program."""

        def __init__(self, name = None, entry = None, priority = None,
                     args = None, stack = None, physpool = None):
            self.name     = name
            self.entry    = entry
            self.priority = priority
            self.args     = args
            self.stack    = stack
            self.physpool = physpool

        def element(self, env = None):
            template = '\t\t<thread name="%s" ' % self.name

            if self.entry:
                template += 'start="%s" ' % self.entry

            if self.priority is not None:
                template += 'priority="%d" ' % self.priority

            if self.physpool:
                template += 'physpool="%s" ' % self.physpool

            template += '>\n'

            if self.stack:
                template += '\t\t\t<stack size="0x%x" />\n' % self.stack
#
            if self.args is not None:
                template += '\t\t\t<commandline>\n'

                for a in self.args:
                    template += '\t\t\t\t<arg value="%s"/>\n' % a

                template += '\t\t\t</commandline>\n'

            template += '\t\t</thread>\n'

            return template

    class _WeaverElement(object):
        def __init__(self, addressing):
            self.initialised = False
            self.segments    = []

            if addressing is None:
                self.addressing = KengeEnvironment._WeaverAddressing(self.__machine__)
            else:
                self.addressing = addressing

        def set_node(self, node, xml_name):
            if isinstance(node, SCons.Node.NodeList):
                node = node[0]

            self.node = node
            self.xml_name = xml_name

        def init(self, initialised=True):
            addressing = self.addressing
            if not addressing.ignore_virt():
                self.base_vaddr = eval(open(self.node.abspath + '.linkaddress', "r").read())

            self.initialised = initialised

        def get_addressing(self):
            return self.addressing

        def get_segments(self):
            if not self.initialised:
                self.init()

            return self.segments

        def get_last_virtual_addr(self):
            assert 0, "get_last_virtual_addr must be defined!"
            
        def element(self, env = None):
            assert 0, "element must be defined!"

        def subnodes(self):
            return []

    class _WeaverIguanaObject(_WeaverElement):
        def __init__(self, addressing):
            KengeEnvironment._WeaverElement.__init__(self, addressing)
            self.vaddr = 0

        def get_last_virtual_addr(self):
            if not self.initialised:
                self.init()

            return self.addressing.align_zone(self.vaddr)


    class _WeaverMemoryPool(_WeaverIguanaObject):
        def __init__(self, src = None, memory = None, zones = None):
            addressing = \
                       KengeEnvironment._WeaverAddressing(
                self.__machine__,
                virt_addr = KengeEnvironment._WeaverAddressing.ADDR_IGNORE,
                phys_addr = KengeEnvironment._WeaverAddressing.ADDR_IGNORE)

            KengeEnvironment._WeaverIguanaObject.__init__(self, addressing)
            self.src = src
            self.zones = zones

            if memory is None:
                self.memory = None
            else:
                self.memory = memory[:]

        def add_zones(self, zones):
            self.zones = zones

        def element(self, env = None):
            if not self.initialised:
                self.init()

            if self.pool_tag() != 'physical_pool':
                text = '<%s name="%s">\n' % (self.pool_tag(), self.xml_name)
            else:
                text = '<%s name="%s" direct="true">\n' % (self.pool_tag(), self.xml_name)

            if self.src is not None:
                text += '\t\t<memory src="%s" />\n' % self.src

            if self.memory is not None:
                for mem in self.memory:
                    text += '\t\t<memory base="0x%x" size="0x%x"/>\n' % \
                            (mem[0], mem[1] - mem[0])

            if self.zones is not None:
                for zone in self.zones:
                    text += '\t\t<zone name="%s" />\n' % zone

            text += '\t</%s>\n' % (self.pool_tag())

            return text
                
    class _WeaverVirtualMemoryPool(_WeaverMemoryPool):
        def pool_tag(self):
            return 'virtual_pool'

    class _WeaverPhysicalMemoryPool(_WeaverMemoryPool):
        def pool_tag(self):
            return 'physical_pool'

    class _WeaverMemSection(_WeaverIguanaObject):
        def __init__(self, base, size, flags, cache_policy, zero = True,
                     addressing = None):
            if addressing is None:
                if base == KengeEnvironment.ADDR_ALLOC_RUNTIME:
                    paddr = KengeEnvironment.ADDR_IGNORE
                else:
                    paddr = KengeEnvironment.ADDR_ALLOC

                addressing = KengeEnvironment._WeaverAddressing(
                    self.__machine__,
                    virt_addr = base,
                    phys_addr = paddr)

            KengeEnvironment._WeaverIguanaObject.__init__(self, 
                                                          addressing)

            self.base     = base
            self.size     = size
            self.flags    = flags
            self.cache_policy = cache_policy
            self.zero     = zero

        def get_last_virtual_addr(self):
            if not self.initialised:
                self.init()

            if self.base != KengeEnvironment.ADDR_ALLOC_RUNTIME:
                return self.addressing.align_zone(self.base_vaddr + self.size)
            else:
                return self.addressing.align_zone(self.base_vaddr)

        def element(self, env = None):
            if not self.initialised:
                self.init()

            # If the address was to be calculated as part of the
            # image, copy it in now.
            if self.base == KengeEnvironment.ADDR_ALLOC:
                self.base = self.base_vaddr

            template = '<memsection name="%s" size="0x%x" ' % \
                       (self.xml_name, self.size)

            if self.base != KengeEnvironment.ADDR_ALLOC_RUNTIME:
                template += 'virt_addr="0x%x" ' % self.base

            if self.addressing.get_virtpool() is not None:
                template += 'virtpool="%s" ' % self.addressing.get_virtpool()

            if self.addressing.get_physpool() is not None:
                template += 'physpool="%s" ' % self.addressing.get_physpool()

            if not self.addressing.has_default_alignment():
                template += 'align="%s" ' % self.addressing.get_virt_align()

            if self.addressing.get_pager() is not None:
                template += 'pager="%s" ' % self.addressing.get_pager()

            if self.addressing.get_direct() is not None:
                template += 'direct="%s" ' % self.addressing.get_direct()

            if not self.zero:
                template += 'zero="false" '

            if self.cache_policy is not None:
                template += 'cache_policy="%s" ' % self.cache_policy

            template += '/>'

            return template

    class _WeaverFile(_WeaverElement):
        def __init__(self, cache_policy = None, addressing = None):
            KengeEnvironment._WeaverElement.__init__(self, addressing)
            self.cache_policy = cache_policy
            self.filename  = None
            self.min_align = self.__machine__.min_alignment

        def get_last_virtual_addr(self):
            if not self.initialised:
                self.init()

            # Files only have one segment.
            seg = self.segments[0]
            return self.addressing.align_zone(self.base_vaddr + seg.get_size())

        def init(self):
            KengeEnvironment._WeaverElement.init(self)
            self.filename = self.node.abspath
            self.vaddr    = eval(open(self.filename + '.linkaddress', "r").read())
            size = align_up(os.stat(self.filename)[ST_SIZE], self.min_align)

            self.segments.append(KengeEnvironment.WeaverELFSegment(size = size,
                                                                   virt_addr = self.base_vaddr))

        def element(self, env = None):
            if not self.initialised:
                self.init()

            # Calculate the XML name of the section.  Use the basename
            # of the file if nothing is specified.
            name = self.xml_name
            if name is None:
                name = os.path.basename(self.filename)

            # Files only have one segment.
            seg = self.segments[0]

            template = '<memsection name="%s" file="%s" size="%d" virt_addr="0x%x" ' % \
                       (name,
                        self.filename, seg.get_size(),
                        seg.get_virt_addr())

            if seg.get_phys_addr() is not None:
                template += 'phys_addr="0x%x" ' % seg.get_phys_addr()

            if self.addressing.get_virtpool() is not None:
                template += 'virtpool="%s" ' % self.addressing.get_virtpool()

            if self.addressing.get_physpool() is not None:
                template += 'physpool="%s" ' % self.addressing.get_physpool()

            if not self.addressing.has_default_alignment():
                template += 'align="%s" ' % self.addressing.get_virt_align()

            if self.addressing.get_pager() is not None:
                template += 'pager="%s" ' % self.addressing.get_pager()

            if self.addressing.get_direct() is not None:
                template += 'direct="%s" ' % self.addressing.get_direct()

            if self.cache_policy is not None:
                template += 'cache_policy="%s" ' % self.cache_policy

            template += '/>'
            return template

    class _WeaverElfFile(_WeaverFile):
        def __init__(self, patches = None, segments = None,
                     addressing = None):
            KengeEnvironment._WeaverFile.__init__(self,
                                                  cache_policy = None,
                                                  addressing = addressing)

            if patches is None:
                self.patches = []
            else:
                self.patches = patches[:]

            if segments is None:
                self.segment_descs = []
            else:
                self.segment_descs = segments[:]

        def init(self, initialised=True):
            KengeEnvironment._WeaverFile.init(self)
            elffile = PreparedElfFile(filename=self.filename)

            # Place loadable segments into the segment table.
            segs = []
            for seg in elffile.segments:
                if seg.type == PT_LOAD:
                    self.segments.append(KengeEnvironment.WeaverELFSegment(elfsegment = seg,
                                                                           phys_from_elf = self.addressing.use_phys_as_built(),
                                                                           physpool = self.addressing.get_physpool()))

            self.last_virtual = elffile.get_last_addr("virtual")
            self.initialised = initialised

        def get_last_virtual_addr(self):
            if not self.initialised:
                self.init()
            return self.addressing.align_zone(self.last_virtual)

        def segment_tags(self):
            """Generate the segment sub-tags for an element."""
            template = ""

            if self.segment_descs is not None:
                for desc in self.segment_descs:
                    template += desc.element()

            return template

        def patch_tags(self):
            """Generate the patch sub-tags for an element."""
            template = ""

            if self.patches is not None:
                for patch in self.patches:
                    template += '\t\t<patch address="%s" value="0x%x" ' % \
                                (patch.address, patch.value)

                    if patch.bytes is not None:
                        template += 'bytes="%d" ' % patch.bytes

                    template += '/>\n'

            return template

        def add_patch(self, address, value, bytes=None):
            patch = KengeEnvironment.WeaverPatch(address, value, bytes)
            self.patches.append(patch)

    class _WeaverKernel(_WeaverElfFile):
        def __init__(self, xip = False, heap = None, patches = None,
                     segments = None, addressing = None, max_threads = 0x1000,
                     max_root_caps = 1024):
            if addressing is None:
                if env.machine.platform == "pc99":
                    kernel_phys_align = KengeEnvironment._WeaverAddressing.ALIGN_MIN
                    kernel_phys_addr = KengeEnvironment._WeaverAddressing.ADDR_AS_BUILT
                else:
                    kernel_phys_align = KengeEnvironment._WeaverAddressing.ALIGN_ELF
                    kernel_phys_addr = KengeEnvironment._WeaverAddressing.ADDR_ALLOC

                addressing = \
                KengeEnvironment._WeaverAddressing(
                    self.__machine__,
                    xip = xip,
                    phys_align = kernel_phys_align,
                    virt_addr = 0x0,
                    phys_addr = kernel_phys_addr)

            KengeEnvironment._WeaverElfFile.__init__(self, patches,
                                                     segments, addressing)
            self.kernel_heap = heap
            self.max_threads = max_threads
            self.max_root_caps = max_root_caps

        def element(self, env = None):
            if not self.initialised:
                self.init()

            template = '<kernel file="%s" xip="%s" ' % (self.filename, xmlbool(self.addressing.do_xip()))
            
            if self.addressing.get_physpool() is not None:
                template += 'physpool="%s" ' % self.addressing.get_physpool()

            template += '>\n'

            # List of the location of the program segments.
            template += self.segment_tags()

            template += self.patch_tags()

            if self.kernel_heap:
                template += '\t\t<heap size="0x%x" ' % self.kernel_heap

                if hasattr(self, 'heap_base'):
                    template += 'phys_addr="0x%x" ' % self.heap_base

                template += '/>\n'
            template += '\t\t<dynamic max_threads="0x%x" />\n' % self.max_threads

            # set max number of roots caps.
            template += '\t\t<config>\n'
            template += '\t\t\t<option key="%s" value="%d"/>\n' % ("root_caps", self.max_root_caps)
            template += '\t\t</config>\n'

            template += '\t</kernel>\n'

            return template

    class _WeaverRootProgram(_WeaverElfFile):
        def __init__(self, stack = None, heap = None, patches = None,
                     segments = None, addressing = None):
            if addressing is None:
                addressing = \
                KengeEnvironment._WeaverAddressing(
                    self.__machine__,
                    virt_addr = KengeEnvironment._WeaverAddressing.ADDR_BASE)

            KengeEnvironment._WeaverElfFile.__init__(self,
                                                     patches,
                                                     segments,
                                                     addressing)
            self.extensions = []
            self.stack      = stack
            self.heap       = heap

        def set_default_pools(self, virtpool, physpool):
            self.default_virtpool = virtpool
            self.default_physpool = physpool

        def add_extension(self, extn):
            if isinstance(extn, SCons.Node.NodeList):
                extn = extn[0]

            self.extensions.append(extn)
            self.node.get_build_env().Depends(self.node, extn)

        def element(self, env = None):
            if not self.initialised:
                self.init()

            # Default pools can be optional if declared so in the constructor.
            if hasattr(self, "default_virtpool") and hasattr(self, "default_physpool"):
                template = '<rootprogram file="%s" virtpool="%s" physpool="%s" ' % \
                           (self.filename, self.default_virtpool,
                            self.default_physpool)
            else:
                raise UserError, "Default Memory Pools have not specified for the Root Program."

            if self.addressing.get_pager() is not None:
                template += 'pager="%s" ' % self.addressing.get_pager()

            if self.addressing.get_direct() is not None:
                template += 'direct="%s" ' % self.addressing.get_direct()

            template += '>\n'

            if self.stack:
                template += '\t\t<stack size="0x%x" />\n' % self.stack

            if self.heap:
                template += '\t\t<heap size="0x%x" />\n' % self.heap

            # List of the location of the program segments.
            template += self.segment_tags()

            template += self.patch_tags()

            for e in self.extensions:
                template += '\t\t' + e.attributes.weaver.element() + '\n'

            template += '\t</rootprogram>\n'

            return template

    class _WeaverIguanaExtension(_WeaverElfFile):
        def __init__(self, patches = None, segments = None,
                     addressing = None):
            KengeEnvironment._WeaverElfFile.__init__(self,
                                                     patches,
                                                     segments,
                                                     addressing)

        def element(self, env = None):
            if not self.initialised:
                self.init()

            # Calculate the XML name of the section.  Use the basename
            # of the file if nothing is specified.
            name = self.xml_name
            if name is None:
                name = os.path.basename(self.filename)

            template = '<extension name="%s" file="%s" ' % \
                       (name, self.filename)

            if self.addressing.get_physpool() is not None:
                template += 'physpool="%s" ' % self.addressing.get_physpool()

            template += '>\n'

            # List of the location of the program segments.
            template += self.segment_tags()

            template += self.patch_tags()

            template += '\t</extension>'

            return template

    class _WeaverIguanaProgram(_WeaverElfFile):
        def __init__(self, priority = None, args = None, stack = None,
                     heap = None, patches = None, extra_threads = None,
                     segments = None, addressing = None,
                     heap_user_map = None, platform_control = False,
                     env_consts = None):
            KengeEnvironment._WeaverElfFile.__init__(self,
                                                     patches,
                                                     segments,
                                                     addressing)

            self.priority    = priority
            self.args        = args
            self.stack       = stack
            self.heap        = heap
            self.ms          = []
            self.extern_objs = []
            self.heap_user_map = heap_user_map
            self.plat_ctrl   = platform_control
            self.zones       = []

            if extra_threads is None:
                self.threads = []
            else:
                self.threads = extra_threads[:]

            if env_consts is None:
                self.env_consts = []
            else:
                self.env_consts = env_consts

        def thread_tags(self):
            """Generate the thread sub-tags for an element."""
            template = ""

            if self.threads is not None:
                for thread in self.threads:
                    template += thread.element()

            return template

        def add_memsection(self, ms):
            if isinstance(ms, SCons.Node.NodeList):
                ms = ms[0]

            self.ms.append(ms)
            self.node.get_build_env().Depends(self.node, ms)

        def add_external_object(self, key, ms, attach=None, str=None):
            if isinstance(ms, SCons.Node.NodeList):
                ms = ms[0]

            self.extern_objs.append((key, ms, attach, str))
            self.node.get_build_env().Depends(self.node, ms)

        def add_zone(self, zone):
            self.zones.append(zone)

            for ms in zone.subnodes():
                self.node.get_build_env().Depends(self.node, ms)

        def subnodes(self):
            """
            Return all memsections that are owned by the program.
            """

            nodes = [z.subnodes() for z in self.zones]
            return nodes + [self.ms]

        def element(self, env = None):
            if not self.initialised:
                self.init()

            # Calculate the XML name of the section.  Use the basename
            # of the file if nothing is specified.
            name = self.xml_name
            if name is None:
                name = os.path.basename(self.filename)

            template = '<program name="%s" file="%s" ' % \
                       (name, self.filename)

            if self.priority is not None:
                template += 'priority="%d" ' % self.priority

            if self.addressing.get_virtpool() is not None:
                template += 'virtpool="%s" ' % self.addressing.get_virtpool()

            if self.addressing.get_physpool() is not None:
                template += 'physpool="%s" ' % self.addressing.get_physpool()

            if self.addressing.get_pager() is not None:
                template += 'pager="%s" ' % self.addressing.get_pager()

            if self.addressing.get_direct() is not None:
                template += 'direct="%s" ' % self.addressing.get_direct()

            if hasattr(self, 'server_name'):
                template += 'server="%s" ' % self.server_name

            if self.plat_ctrl:
                str = __builtins__["str"] # MAGIC workaround of broken SCons
                template += 'platform_control="%s" ' % str(self.plat_ctrl).lower()

            template += '>\n'

            if self.stack:
                template += '\t\t<stack size="0x%x" />\n' % self.stack

            if self.heap:
                template += '\t\t<heap size="0x%x"' % self.heap
                if self.heap_user_map:
                    template += ' user_map="true"'
                template += ' />\n'



            # List of the location of the program segments.
            template += self.segment_tags()

            template += self.patch_tags()

            template += self.thread_tags()

            for zone in self.zones:
                template += zone.element()

            if self.args is not None:
                template += '\t\t<commandline>\n'

                for a in self.args:
                    template += '\t\t\t<arg value="%s"/>\n' % a

                template += '\t\t</commandline>\n'

            for a in self.ms:
                template += '\t\t' + a.attributes.weaver.element() + '\n'

            # Find out how many virtual device instances should be declared here
            if env.spawn_vdevs.has_key(name):
                for i in range(env.spawn_vdevs[name]):
                    template += '\t\t<virt_device name="%s%d" />\n' % (name, i)

            # Print out custom entries in the environment.
            need_env = len(self.extern_objs) != 0 or \
                       len(self.env_consts) != 0 or \
                       hasattr(self, 'device_name') or \
                       env.required_vdevs.has_key(name)

            if need_env:
                template += '\t\t<environment>\n'

                if hasattr(self, 'device_name'):
                    res = "_".join([self.device_name, "resource"])
                    dev = "_".join([self.device_name, "dev"])
                    res = res.upper()
                    template += '\t\t\t<entry key="%s" cap="/dev/%s" />\n' % (res, dev)

                if env.required_vdevs.has_key(name):
                    for vdev in env.required_vdevs[name]:
                        template += '\t\t\t<entry key="%s" cap="/dev/%s%d" />\n' % (vdev.upper(), vdev, env.vdev_counter[vdev])
                        env.vdev_counter[vdev] += 1

                for (key, object, attach, str) in self.extern_objs:
                    #template += '\t\t\t<entry key="%s" cap="%s" ' % (key, server)
                    template += '\t\t\t<entry key="%s" cap="%s" ' % (key, str)

                    if attach is not None:
                        template += 'attach="%s" ' % attach

                    template += '/>\n'

                for (key, value) in self.env_consts:
                    template += '\t\t\t<entry key="%s" value="%d" />\n' % (key, value)

                template += '\t\t</environment>\n'

            template += '\t</program>\n'
            return template

    class _WeaverIguanaProgramServer(_WeaverIguanaProgram):
        def __init__(self, server_name, priority = None,
                     args = None, stack = None, heap = None,
                     patches = None, extra_threads = None,
                     segments = None, addressing = None,
                     device_name = None, heap_user_map = None,
                     platform_control = False, env_consts = None):
            KengeEnvironment._WeaverIguanaProgram.__init__(self,
                                                           priority, args,
                                                           stack,
                                                           heap,
                                                           patches,
                                                           extra_threads,
                                                           segments,
                                                           addressing,
                                                           heap_user_map,
                                                           platform_control,
                                                           env_consts)
            self.server_name = server_name
            if device_name is not None:
                self.device_name = "%s" % device_name

        def set_node(self, node, xml_name):
            KengeEnvironment._WeaverIguanaProgram.set_node(self, node, xml_name)

    # SConscript interface functions.  Wrap the classes in the
    # enclosing Environment instance.  Weaver attributes are added to
    # image targets using the form:
    #
    # env.KengeProgram(.....
    #                  weaver = env.WeaverFoo(...)
    #                 )

    WeaverVirtualMemoryPool   = env_inner_class(_WeaverVirtualMemoryPool)
    WeaverPhysicalMemoryPool  = env_inner_class(_WeaverPhysicalMemoryPool)
    WeaverMemSection          = env_inner_class(_WeaverMemSection)
    WeaverThread              = env_inner_class(_WeaverThread)
    WeaverFile                = env_inner_class(_WeaverFile)
    WeaverKernel              = env_inner_class(_WeaverKernel)
    WeaverRootProgram         = env_inner_class(_WeaverRootProgram)
    WeaverIguanaExtension     = env_inner_class(_WeaverIguanaExtension)
    WeaverIguanaProgram       = env_inner_class(_WeaverIguanaProgram)
    WeaverIguanaProgramServer = env_inner_class(_WeaverIguanaProgramServer)
    # For anyone who really likes elfweaver.
    WeaverPhile               = WeaverFile


    ALIGN_MIN          = _WeaverAddressing.ALIGN_MIN
    ALIGN_PREFERRED    = _WeaverAddressing.ALIGN_PREFERRED
    ALIGN_SUPERPAGE    = _WeaverAddressing.ALIGN_SUPERPAGE
    ALIGN_ELF          = _WeaverAddressing.ALIGN_ELF
    ADDR_BASE          = _WeaverAddressing.ADDR_BASE
    ADDR_ALLOC         = _WeaverAddressing.ADDR_ALLOC
    ADDR_AS_BUILT      = _WeaverAddressing.ADDR_AS_BUILT
    ADDR_IGNORE        = _WeaverAddressing.ADDR_IGNORE
    ADDR_ALLOC_RUNTIME = _WeaverAddressing.ADDR_ALLOC_RUNTIME

    def WeaverAddressing(self,
                         xip        = False,
                         virt_align = ALIGN_PREFERRED,
                         phys_align = ALIGN_SUPERPAGE,
                         virt_addr  = ADDR_ALLOC,
                         phys_addr  = ADDR_ALLOC,
                         virtpool   = None,
                         physpool   = None,
                         pager      = None,
                         direct     = None,
                         size_hint  = 0x10000):

        return self._WeaverAddressing(self.machine,
                                      xip        = xip,
                                      virt_align = virt_align,
                                      phys_align = phys_align,
                                      virt_addr  = virt_addr,
                                      phys_addr  = phys_addr,
                                      virtpool   = virtpool,
                                      physpool   = physpool,
                                      pager      = pager,
                                      direct     = direct,
                                      size_hint  = size_hint)


# Option processing functions.

def get_bool_arg(arg, default):
    """Get the boolean value of an option."""
    global build
    x = build.extra_args.get(arg, default)
    if type(x) is type(""):
        x.lower()
        if x in [1, True, "1", "true", "yes"]:
            x = True
        elif x in [0, False, "0", "false", "no"]:
            x = False
        else:
            raise UserError, "%s is not a valid argument for option %s. It should be True or False" % (x, arg)

    return x

def get_option_arg(name, default, options = None, as_list = False):
    """Get the value of an option, which must be one of the supplied set"""
    global build
    def_val = default
    arg = build.extra_args.get(name, def_val)
    optstr = ""
    mapping = identity_mapping

    if options and type(options[0]) == TupleType:
        # This means a user has specified a list like:
        # [("foo", foo_object), ("bar", bar_object)]
        mapping = dict(options)
        options = [x[0] for x in options]

    if options and arg == None:
        if arg not in options:
            raise UserError, "need argument for option %s. Valid options are: %s" % (name, options)
        return mapping[arg]
    else:
        if options:
            if not isinstance(arg, str): #Assuming integer
                if arg not in options:
                    raise UserError, "%s is not a valid argument for option %s. Valid options are: %s" % (x, name, options)
                return arg
            arg = arg.split(",")
            for x in arg:
                if x not in options:
                    raise UserError, "%s is not a valid argument for option %s. Valid options are: %s" % (x, name, options)

            if as_list:
                return arg
            else:
                return mapping[arg[0]]

            optstr = " Valid options are:\n\t\t%s" % list2str(options)
        else:
            return mapping[arg]

def get_rtos_example_args(xml_file, example_cmd_line):
    """Read the given rtos tests configuration xml file and return a dictionary 
       containing the values read"""
    rtos_args = {}
    for example in example_cmd_line:
        if not example == "all":
            rtos_args[example] = {}
            rtos_args[example]["nb_copies"] = 1
            rtos_args[example]["extra_arg"] = 1
    xmltree = minidom.parse("tools/unittest/" + xml_file).documentElement
    rtos_args["server"] = xmltree.getAttribute("server")
    example_nodes = xmltree.getElementsByTagName("example")
    for example_node in example_nodes:
        example_name = example_node.getAttribute("name")
        if example_name in example_cmd_line or example_cmd_line == ["all"]:
            if example_cmd_line == ["all"]:
                rtos_args[example_name] = {}
            rtos_args[example_name]["nb_copies"] = example_node.getAttribute("nb_copies")
            rtos_args[example_name]["extra_arg"] = example_node.getAttribute("extra_arg")
    return rtos_args

################################################################################
# Now find the project SConstruct file to use.
################################################################################

# Find the project from the arguments.
project = conf.dict.get("PROJECT", None)
if "PROJECT" in conf.dict: del conf.dict["PROJECT"]

# Provide useful message if it doesn't exist
if project is None:
    raise  UserError, "You must specify the PROJECT you want to build."

cust = conf.dict.get('CUST', None)
sconstruct_path = "cust/%s/projects/%s/SConstruct" % (cust, project)
if not os.path.exists(sconstruct_path):
    sconstruct_path = "projects/%s/SConstruct" % project
    if not os.path.exists(sconstruct_path):
        raise UserError, "%s is not a valid project" % project

# Execute the SConstruct
execfile(sconstruct_path)
