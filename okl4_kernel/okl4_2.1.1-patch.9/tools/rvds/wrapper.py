#!/usr/bin/python
#
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
#

#
# Authour: Philip O'Sullivan <philipo@ok-labs.com>
# Date   : Mon 19 Feb 2007
#

from cStringIO import StringIO
import os
import re
import subprocess
import sys
import tempfile
import time

class Config:
    def __init__(self, toolchain, path, warnings, licence):
        self.toolchain = toolchain
        self.path = path
        self.warnings = warnings
        self.licence = licence

def main():
    if len(sys.argv) < 4:
        print "Usage: %s <toolchain> <command> <arg> [<arg> ...]" % sys.argv[0]
        sys.exit(1)

    toolchain, cmd = sys.argv[1:3]
    if toolchain == 'ads':
        warnings = re.compile(r'(.*line \d+: Warning: .*:.*|.*Finished: [0-9]+ information, [1-9][0-9]* warning and [0-9]+ error messages.)', re.MULTILINE | re.DOTALL)
        licence = re.compile('(Serious |Fatal |)[Ee]rror: .*: (Licensed number of users already reached|Cannot obtain license for.*)')
        config = Config(toolchain, '/opt/arm/linux/bin', warnings, licence)
    elif toolchain == 'rvct':
        warnings = re.compile('^.*Warning: .*: .*$')
        licence = re.compile(r'.*Error: (C3397E|A1439E): Cannot obtain license for.*', re.MULTILINE | re.DOTALL)
        config = Config(toolchain, '/opt/rvds/RVCT/Programs/2.2/503/linux-pentium', warnings, licence)
    run_cmd(config, cmd, attempts=20)

def clean_output(txt):
    lines = txt.split('\n')
    msg = 'Waiting for license...'
    result = []
    for line in lines:
        if line != msg:
            result.append(line)
    return '\n'.join(result)


def run_cmd(config, cmd, attempts=1):
    cmd = os.path.join(config.path, cmd)
    cmd_line = [cmd] + sys.argv[3:]

    for attempt in range(1,attempts+1):
        log = tempfile.TemporaryFile()
        try:
            child = subprocess.Popen(cmd_line, stdin=sys.stdin, stdout=log, stderr=subprocess.STDOUT)
            retcode = child.wait()
        except OSError, ex:
            # Print an error message, rather than an traceback, if the
            # command cannot be found.
            print ex
            sys.exit(127)
            
        log.seek(0)
        output = clean_output(log.read())
        if retcode:
            if config.licence.search(output):
                print >>sys.stderr, 'retrying ...'
                time.sleep(attempt*2)
                continue
            # We have a normal compile error 
            print output,
            if retcode > 127:
                retcode = 127
            sys.exit(retcode)
        if config.warnings.search(output):
            print output,
            sys.exit(1)
        if output:
            print output,
        sys.exit(0)
    print >>sys.stderr, '%s: unable to obtain licence' % cmd
    sys.exit(1)

if __name__ == '__main__':
    main()
