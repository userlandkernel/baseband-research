#
# Australian Public Licence B (OZPLB)
# 
# Version 1-0
# 
# Copyright (c) 2004 National ICT Australia
# 
# All rights reserved. 
# 
# Developed by: Embedded Real-time and Operating Systems Group (ERTOS)
#               National ICT Australia
#               http://www.ertos.nicta.com.au
# 
# Permission is granted by National ICT Australia, free of charge, to
# any person obtaining a copy of this software and any associated
# documentation files (the "Software") to deal with the Software without
# restriction, including (without limitation) the rights to use, copy,
# modify, adapt, merge, publish, distribute, communicate to the public,
# sublicense, and/or sell, lend or rent out copies of the Software, and
# to permit persons to whom the Software is furnished to do so, subject
# to the following conditions:
# 
#    # Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimers.
# 
#    # Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimers in the documentation and/or other materials provided
#       with the distribution.
# 
#    # Neither the name of National ICT Australia, nor the names of its
#       contributors, may be used to endorse or promote products derived
#       from this Software without specific prior written permission.
# 
# EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
# PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
# NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
# WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
# BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
# REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
# THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
# ERRORS, WHETHER OR NOT DISCOVERABLE.
# 
# TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
# NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
# THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
# LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
# OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
# OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
# OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
# CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
# CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
# DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
# CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
# DAMAGES OR OTHER LIABILITY.
# 
# If applicable legislation implies representations, warranties, or
# conditions, or imposes obligations or liability on National ICT
# Australia or one of its contributors in respect of the Software that
# cannot be wholly or partly excluded, restricted or modified, the
# liability of National ICT Australia or the contributor is limited, to
# the full extent permitted by the applicable legislation, at its
# option, to:
# a.  in the case of goods, any one or more of the following:
# i.  the replacement of the goods or the supply of equivalent goods;
# ii.  the repair of the goods;
# iii. the payment of the cost of replacing the goods or of acquiring
#  equivalent goods;
# iv.  the payment of the cost of having the goods repaired; or
# b.  in the case of services:
# i.  the supplying of the services again; or
# ii.  the payment of the cost of having the services supplied again.
# 
# The construction, validity and performance of this licence is governed
# by the laws in force in New South Wales, Australia.
#

# Turn warnings into errors because we care.
import warnings, errno
warnings.filterwarnings('error')


VERSION_NUMBER = 11 # Monotonically-increasing integer
VERSION_DATES = {1: 'August 25 2004',
	2: 'November 11 2004',
	3: 'December 12 2004',
	4: 'January 19 2005',
	5: 'February 15 2005',
	6: 'March 4 2005',
	7: 'May 23 2005', # Public release
	8: 'Nov 6 2005',
	# Versions 9 and 10 skipped due to baz
	11: 'Apr 23 2007 (Algorab)',}

VERSION_STRING = "Magpie %d, %s" % (VERSION_NUMBER, VERSION_DATES[VERSION_NUMBER])

import platform
import os.path
import sys
import re

def get_module_base_path():
	return os.path.split(__file__)[0]

SANITY_VERSION_ERROR = """Please use a more recent version of Python.
Magpie requires at least Python version 2.3. You are using Python %d.%d.%d.""" % \
		(sys.version_info[0], sys.version_info[1], sys.version_info[2])
def sanity_checks():
	# Sanity checks that should be performed before we get started.
	# ... Python version
	if sys.version_info[0] * 100 + sys.version_info[1] < 203:
		print >>sys.stderr, SANITY_VERSION_ERROR
		sys.exit(1)

# String-related helpers
def c_friendly_string(string):
	""" Return a string where most "non-C" characters are replaced
	with underscores."""
	return re.sub(r'\.|/|\!|\@|\#|\$|\%|\^|\&|\*|\(|\)|\~|\`|\'|^[1234567890]', '_', string)

# path-related stuff
def get_storage_directory():
	if "Windows" in platform.system():
		# FIXME: Is this the best way to find out if we're running on Windows?
		return os.path.join(get_module_base_path(), 'storage')
	else:
		# FIXME: More test cases required here.
		return os.path.join(os.environ['HOME'], '.magpie')
	raise Exception("Unable to determine home directory")

def get_magpie_dir(dir_type, options):
	if options and options[dir_type] is not None:
		return options[dir_type]
	if dir_type == 'cache_dir':
		# Default spot is $HOME/.magpie/cache/
		conf_dir = os.path.join(get_storage_directory(), 'cache')
	elif dir_type == 'template_cache_dir':
		conf_dir = os.path.join(get_storage_directory(), 'templates')
	else:
		raise Exception("Unknown configuration directory type")
	# Create the directory if it doesn't exist.
	try:
		os.makedirs(conf_dir)
	except OSError, e:
		if e.errno != errno.EEXIST:
			raise
	return conf_dir
	
def get_OSPath(path_list):
	path = ''
	for directory in path_list:
		path = os.path.join(path, directory)
	return path
# Stupid python 2.3 broken FutureWarning
# The following comes from http://mail.python.org/pipermail/python-dev/2005-February/051560.html 
# Addresses can "look negative" on some boxes, some of the time.  If you
# feed a "negative address" to an %x format, Python 2.3 displays it as
# unsigned, but produces a FutureWarning, because Python 2.4 will display
# it as signed.  So when you want to prodce an address, use positive_id() to
# obtain it.
def positive_id(obj):
	"""Return id(obj) as a non-negative integer."""

	result = id(obj)
	if result < 0:
		# This is a puzzle:  there's no way to know the natural width of
		# addresses on this box (in particular, there's no necessary
		# relation to sys.maxint).  Try 32 bits first (and on a 32-bit
		# box, adding 2**32 gives a positive number with the same hex
		# representation as the original result).
		result += 1L << 32
		if result < 0:
			# Undo that, and try 64 bits.
			result -= 1L << 32
			result += 1L << 64
			assert result >= 0 # else addresses are fatter than 64 bits
	return result

def usr_bin_which(filename):
	# Given a filename, return the full path to that file.
	if filename:
		for path in os.environ.get('PATH').split(os.pathsep):
			full_path = os.path.join(path, filename)
			if os.access(full_path, os.X_OK):
				return full_path
	return None


class odict(dict):

    def __init__(self, d={}):
        self._keys = d.keys()
        dict.__init__(self, d)

    def __delitem__(self, key):
        dict.__delitem__(self, key)
        self._keys.remove(key)

    def __setitem__(self, key, item):
        dict.__setitem__(self, key, item)
        # a peculiar sharp edge from copy.deepcopy
        # we'll have our set item called without __init__
        if not hasattr(self, '_keys'):
            self._keys = [key,]
        if key not in self._keys:
            self._keys.append(key)

    def clear(self):
        dict.clear(self)
        self._keys = []

    def items(self):
        for i in self._keys:
            yield i, self[i]

    def keys(self):
        return self._keys

    def popitem(self):
        if len(self._keys) == 0:
            raise KeyError('dictionary is empty')
        else:
            key = self._keys[-1]
            val = self[key]
            del self[key]
            return key, val

    def setdefault(self, key, failobj = None):
        dict.setdefault(self, key, failobj)
        if key not in self._keys:
            self._keys.append(key)

    def update(self, d):
        for key in d.keys():
            if not self.has_key(key):
                self._keys.append(key)
        dict.update(self, d)

    def values(self):
        for i in self._keys:
            yield self[i]

    def move(self, key, index):

        """ Move the specified to key to *before* the specified index. """

        try:
            cur = self._keys.index(key)
        except ValueError:
            raise KeyError(key)
        self._keys.insert(index, key)
        # this may have shifted the position of cur, if it is after index
        if cur >= index: cur = cur + 1
        del self._keys[cur]

    def index(self, key):
        if not self.has_key(key):
            raise KeyError(key)
        return self._keys.index(key)

