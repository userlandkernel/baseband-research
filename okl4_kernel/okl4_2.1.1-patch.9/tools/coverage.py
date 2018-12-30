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
# Author: Alex Webster

import os
import sys
import time
from optparse import OptionParser

output_filename = "_test_coverage.txt"
usage = "usage: %prog -p packages_list [options]"
parser = OptionParser(usage)
parser.add_option("-p", "--packages", dest="packages_list", default='',
                  help="Packages to assess (separated by comma)")

parser.add_option("-b", "--build_dir", dest="build_dir", default="build",
                  help="Directory where are located the binaries (default %default)")

parser.add_option("-t", "--tracelog_file", dest="tracelog_file", default="/tmp/sk1.log",
                  help="Absolute path to the skyeye tracelog file (default %default)")

parser.add_option("-f", "--file", action="store_true", dest="output_file", default=False,
                  help="Write output to file \"<package_name>%s\" for each package specified with -p option (default False)" % output_filename)

parser.add_option("-a", "--all", action="store_true", dest="all_symbols", default=False,
                  help="Print address coverage for all symbols including those not in the packages specified with -p option (default %default)")

parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False,
                  help="Print progress information (default %default)")

parser.add_option("-s", "--summary", action="store_true", dest="summary", default=False,
                  help="Print a summary of the results (default %default)")

(options, args) = parser.parse_args()

if not options.packages_list:
    parser.error("No package specified")
src_dirs = options.packages_list.split(",")
output = {}
if options.all_symbols:
    all_src_dirs = src_dirs + ["others"]
else:
    all_src_dirs = src_dirs
for src_dir in all_src_dirs:
    pkg_name = src_dir.replace("/", "")
    if options.output_file:
        output[src_dir] = open(os.path.join(options.build_dir, pkg_name + output_filename), "w")
    else:
        output[src_dir] = sys.stdout

if "l4_test" in all_src_dirs or "pistachio" in all_src_dirs:
    l4test = 1
    testlib = 0
else:
    l4test = 0
    testlib = 1

binaries = []
for d in os.walk(options.build_dir).next()[1]:
    if l4test and d.find("pistachio") == -1 and d.find("l4") == -1:
        continue
    if testlib and d.find("iguana") == -1:
        continue
    dirname = os.path.join(options.build_dir, d, "bin")
    for f in os.listdir(dirname):
        pathname = os.path.join(dirname, f)
        if "executable" in os.popen2("file %s" % pathname)[1].read():
            if testlib:
                if f == "test" or f == "ig_server":
                    binaries.append(pathname)
            else:
                binaries.append(pathname)

if options.verbose: print "Scanning symbols..."
lookup = {}
sym2package = {}
count = 0
last = 0

def find_addr2skip(binary):
    addr2skip = {}
    for line in os.popen2("arm-linux-objdump -h %s" % binary)[1]:
        fields = line.split()
        if len(fields) >= 4 and \
           (fields[1].find(".kdebug") != -1 or fields[1] == ".init"):
            addr2skip[fields[1]] = (int(fields[3], 16), int(fields[3], 16) + int(fields[2], 16))

    return addr2skip

def skip_addr(addr, addr2skip):
    for section, range in addr2skip.iteritems():
        start, end = range
        if addr >= start and addr <= end:
            return True
    return False

for bin in binaries:
    if options.verbose: print "%s..." % bin,
    if bin.find("pistachio") != -1:
        addr2skip = find_addr2skip(bin)
    for line in os.popen2("nm --size-sort -S %s" % bin)[1]:
        fields = line.split()
        value = int(fields[0], 16)
        size = int(fields[1], 16)
        name = fields[3]

        p = os.popen2("addr2line -Cfi -e %s" % bin)
        for addr in range(value, value + size, 4):
            if bin.find("pistachio") != -1 and skip_addr(addr, addr2skip):
                continue
            p[0].write("%x\n" % addr)
            p[0].flush()
            sym = p[1].readline().strip()
            debug = p[1].readline().strip()
            if sym in sym2package and sym2package[sym] != "others" and debug.split(":")[0].endswith(".h"):
                lookup[addr] = (sym, debug.split(":")[0], debug.split(":")[1])
            else:
                for package in src_dirs:
                    #if debug.startswith(package + '/'):
                    if debug.find(package + '/') != -1:
                        if sym not in sym2package:
                            sym2package[sym] = package
                        else:
                            sym2package[sym] += ", " + package
                        lookup[addr] = (sym, debug.split(":")[0], debug.split(":")[1])
                if options.all_symbols and addr not in lookup:
                    if sym not in sym2package:
                        sym2package[sym] = "others"
                    else:
                        sym2package[sym] += ", others"
                    lookup[addr] = (sym, debug.split(":")[0], debug.split(":")[1])
            count += 1
            if time.time() - last > 1:
                if options.verbose: print "%d..." % count,
                sys.stdout.flush()
                last = time.time()
        p[0].close()
        p[1].close()
    if options.verbose: print "\n",
if options.verbose: print "Finished scanning symbols..."

if options.verbose: print "Searching for source files..."
source = {}
packages_result = {}
for src_dir in src_dirs:
    packages_result[src_dir] = []
    if not os.path.isdir(src_dir):
        print "Warning: %s is not a directory!" % src_dir
        continue
    for root, dirs, files in os.walk(src_dir):
        for filename in files:
            pathname = os.path.join(root, filename)
            if (os.path.splitext(pathname)[1] == ".c" or os.path.splitext(pathname)[1] == ".cc") \
                and pathname.find("{arch}") == -1 and pathname.find("kdb") == -1:
                source[pathname] = {}
if options.verbose: print "Finished searching for source files..."

syms = {}
for addr, info in lookup.iteritems():
    sym, filename, lineno = info
    if sym not in syms:
        syms[sym] = {}
    syms[sym][addr] = False

if options.verbose: print "Reading tracelog file..."
try:
    n = 0
    for line in file(options.tracelog_file):
        addr = long(line, 16)
        if addr in lookup:
            name, filename, lineno = lookup[addr]
            syms[name][addr] = True
            if filename in source:
                if lineno not in source[filename]:
                    source[filename][lineno] = 0
                source[filename][lineno] += 1
        n += 1
        if time.time() - last > 1:
            if options.verbose: print "%d..." % n,
            sys.stdout.flush()
            last = time.time()
except:
    print "Error while reading tracelog file %s!" % options.tracelog_file
    raise
    
if options.verbose: print "\nFinished reading tracelog file..."

cov_summary = {}
if options.all_symbols:
    cov_summary["others"] = ""
def print_result(packages_result, pkg_average_cov, coverage_type):
    if not options.output_file:
        print "\n%s coverage:" % coverage_type.capitalize(),
    for pkg, pkg_coverage in sorted(packages_result.items()):
        if options.output_file:
            print >> output[pkg], "\n%s coverage:" % coverage_type.capitalize()
        else:
            print >> output[pkg], "\n  Package %s:" % pkg
        if pkg_average_cov.has_key(pkg):
            hits, total = pkg_average_cov[pkg]
        else:
            print >> output[pkg], "  No %s found for this package" % coverage_type
            if options.summary:
                if not cov_summary.has_key(pkg): cov_summary[pkg] = ""
                cov_summary[pkg] += "  No %s found for this package\n" % coverage_type
            continue
        average_cov = 0
        if total: 
            average_cov = 100.0 * hits / total
        print >> output[pkg], "  Average %s coverage = %.1f%%" % (coverage_type, average_cov)
        if options.summary:
            cov_summary[pkg] += "  Average %s coverage = %.1f%%\n" % (coverage_type, average_cov)
        for elem_coverage in pkg_coverage:
            print >> output[pkg], elem_coverage

if options.verbose: print "Writing results..."
if not options.output_file: print "\n<results>",
# Line coverage
pkg_average_cov = {}
for filename, totals in sorted(source.items()):
    if not filename.startswith("%s/" % package):
        for package in src_dirs:
            if filename.startswith("%s/" % package):
                break
    linecount = len(totals)
    sloc_output = os.popen3("sloccount --details %s" % filename)[1].readlines()
    lastline_out = sloc_output[len(sloc_output) - 1].split()
    linetotal = int(lastline_out[0])
    if package not in pkg_average_cov:
        pkg_average_cov[package] = (0, 0)
    pkg_average_cov[package] = (pkg_average_cov[package][0] + linecount, pkg_average_cov[package][1] + linetotal)
    percent = 0
    if linetotal:
        percent = 100.0 * linecount / linetotal
    packages_result[package].append("     %s: %.1f%%" % (filename.replace("%s/" %os.getcwd(), ""), percent))
    if options.summary: cov_summary[package] = ""
print_result(packages_result, pkg_average_cov, "line")

def in_packages_list(lookup, address, package_list):
    for pkg in package_list:
        #if lookup[address][1].startswith(pkg + '/'):
        if lookup[address][1].find(pkg + '/') != -1:
            return True
    return False

# Address coverage
for i in packages_result:
    packages_result[i] = []
if options.all_symbols:
    packages_result["others"] = []
pkg_average_cov = {}
for sym in sorted(syms.keys()):
    all_hits = [addr for addr, ok in syms[sym].items() if ok]
    all_misses = [addr for addr, ok in syms[sym].items() if not ok]
    if sym in sym2package:
        pkg_list = set(sym2package[sym].split(", "))
        for pkg in pkg_list:
            if options.all_symbols and pkg == "others":
                hits = [addr for addr in all_hits if not in_packages_list(lookup, addr, pkg_list)]
                misses = [addr for addr in all_misses if not in_packages_list(lookup, addr, pkg_list)]
                percent = 100.0 * len(hits) / (len(hits) + len(misses))
                packages_result[pkg].append("     %s: %.1f%%" % (sym, percent))
                #  Results with debugging info
                #files_list = [lookup[addr][1] for addr in hits + misses]
                #files_list = set(files_list)
                #files = ", ".join(files_list)
                #packages_result[pkg].append("     %s: %.1f%%  (%s)\n" % (sym, percent, files))
            else:
                #hits = [addr for addr in all_hits if lookup[addr][1].startswith(pkg + '/')]
                hits = [addr for addr in all_hits if lookup[addr][1].find(pkg + '/') != -1]
                #misses = [addr for addr in all_misses if lookup[addr][1].startswith(pkg + '/')]
                misses = [addr for addr in all_misses if lookup[addr][1].find(pkg + '/') != -1]
                percent = 100.0 * len(hits) / (len(hits) + len(misses))
                packages_result[pkg].append("     %s: %.1f%%" % (sym, percent))
                #  Results with debugging info
                #packages_result[pkg].append("     %s: %.1f%% (%d/%d)" % (sym, percent, len(hits), len(misses)))
                #packages_result[pkg].append("      Symbol defined in %s (percent for all addresses: %d/%d)" % (pkg_list, len(all_hits), len(all_misses)))
            if pkg not in pkg_average_cov:
                pkg_average_cov[pkg] = (0, 0)
            pkg_average_cov[pkg] = (pkg_average_cov[pkg][0] + len(hits), pkg_average_cov[pkg][1] + len(hits) + len(misses))
print_result(packages_result, pkg_average_cov, "address")

if options.output_file:
    for file in output.values():
        file.close()
if not options.output_file: print "</results>"
if options.summary: 
    print "<summary>",
    for package in sorted(cov_summary.keys()):
        print "\nPackage %s:\n%s" % (package, cov_summary[package]),
    print "</summary>"
if options.verbose: print "Finished writing results..."
