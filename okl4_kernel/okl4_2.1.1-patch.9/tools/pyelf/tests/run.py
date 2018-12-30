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

"""A test runner for all the ELF tests."""

import pdb
import os
import sys
import glob
import imp
import unittest
import coverage
import traceback
from optparse import OptionParser

# This is some quite evil magic
base_dir = os.path.split(__file__)[0]

if base_dir:
    os.chdir(base_dir)
try:
    import elf
except:
    sys.path.append(os.path.split(os.getcwd())[0])
    import elf

def start_coverage():
    """Setup coverage"""
    coverage.use_cache(False)
    coverage.erase()
    coverage.start()

def main():
    """Main testing interface"""
    parser = OptionParser(add_help_option=False)
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                      help="Run tests verbosely")
    parser.add_option("-q", "--quiet", action="store_true", dest="quiet",
                      help="Silent output (on success)")
    parser.add_option("-s", "--stop", action="store_true", dest="stop",
                      help="Stop")
    parser.add_option("-d", "--debug", action="store_true", dest="debug",
                      help="Debug (drop into PDB)")
    parser.add_option("-p", "--package", action="append", type="string", dest="package",
                      help="Only run tests for a specific package. (E.g: elf or weaver)")

    (options, args) = parser.parse_args()

    if args == []:
        args = glob.glob("test_*.py")
        args.sort()
    passed = True	# If a test fails set this to false
    for test_file in args:
        test_module = os.path.splitext(test_file)[0]
        fp, pathname, description = imp.find_module(test_module)
        start_coverage()
        module = imp.load_module(test_module, fp, pathname, description)
        result = unittest.TestResult()
        tested_modules = getattr(module, "modules_under_test", [])
        if options.package:
            skip = False
            for test_mod in tested_modules:
                package = test_mod.__name__.split(".")[0]
                if package not in options.package:
                    skip = True
                    break
            if skip:
                continue
        suite = unittest.defaultTestLoader.loadTestsFromModule(module)
        if options.debug:
            try:
                suite.debug()
            except:
                traceback.print_exc()
                _, _, tb = sys.exc_info()
                pdb.post_mortem(tb)
                sys.exit()
        else:
            suite.run(result)

        # Reload the modules under test
        for module in tested_modules:
            bits = module.__name__.split(".")
            module_name = bits[-1]
            if len(bits) > 1:
                path = [os.path.dirname(module.__file__)]
            else:
                path = None
            try:
                fp, pathname, description = imp.find_module(module_name, path)
            except ImportError:
                pdb.set_trace()
                print "Error loading", module, path
            imp.load_module(module_name + "_loaded_for_test", fp, pathname, description)

        coverage.stop()

        # Determine which modules do not have 100% coverage.  Print
        # coverage reports on those modules.
        coverage_print = []

        for mod in tested_modules:
            if len(coverage.analysis(mod)[2]):
                coverage_print.append(mod)

        # Result
        if not result.wasSuccessful() or coverage_print:
            print ">>> %s" % test_file

        if not result.wasSuccessful():
            for _t, err in result.errors + result.failures:
                print "=" * 80
                print _t
                print "=" * 80
                print err
            passed = False

        if coverage_print:
            coverage.report(coverage_print)
            print

    sys.exit(not passed)    # If a test failures return a non-zero return code

if __name__ == "__main__":
    main()
