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

import math

# Architecture-specific mappings for bit sizes.

# TODO: Apparently you can't call static methods while the class in which
# they are defined, is still being defined. This is why this method (_expand)
# isn't part of the BaseTypeSizes class, which is where it should be.
def _expand(dict_in):
	""" Convert {key1: list1, key2: list2} to
	{list1[0]: key1, list1[1]: key1, list2[0]: key2 etc}
	"""
	dict_out = {}
	for key in dict_in:
		for item in dict_in[key]:
			dict_out[item] = key
	return dict_out

class BaseTypeSizes(object):
	TYPE_MAP = _expand({
		80: ['long double'],
		64: ['fpage', 'signed long long int', 'unsigned long long int', 'double', 'signed __int64', 'unsigned __int64'],
		32: ['signed int', 'unsigned int', 'boolean', 'signed long int', 'unsigned long int', 'float'],
		16: ['signed short int', 'unsigned short int', 'wchar_t', 'wchar'],
		8: ['signed char', 'unsigned char', 'octet'],
		1: ['bool'], # FIXME?
		0: ['void', 'string', 'wstring', 'smallstring', '__builtin_va_list'],
	})

	TYPE_RANGE = {'long long': (-math.pow(2, 63), math.pow(2, 63) - 1),
			'long': (-math.pow(2, 31), math.pow(2, 31) -1),
			'short': (-math.pow(2, 15), math.pow(2, 15) -1),
			'int': (-math.pow(2, 31), math.pow(2, 31) - 1),
			'unsigned int': (0, math.pow(2, 32) - 1),
			'unsigned short': (0, math.pow(2, 16) -1),
			'unsigned long': (0, math.pow(2, 32) -1),
			'unsigned long long': (0, math.pow(2, 64) - 1),
			'octet': (0, math.pow(2, 8) -1),
	}
	
	def size_in_bits(self, typename):
		return self.TYPE_MAP[typename]
	
	def smallest(self, typename):
		return self.TYPE_RANGE.get(typename, (None, None))[0]

	def largest(self, typename):
		return self.TYPE_RANGE.get(typename, (None, None))[1]
		
class MIGTypeSize(object):
	TYPE_MAP = _expand({
		64: ["MACH_MSG_TYPE_INTEGER_64",
			"MACH_MSG_TYPE_REAL_64"],
		32: ["polymorphic",
			"MACH_MSG_TYPE_BOOLEAN",
			"MACH_MSG_TYPE_INTEGER_32",
			"MACH_MSG_TYPE_REAL_32" ,
			"MACH_MSG_TYPE_PORT_NAME"],
		16: ["MACH_MSG_TYPE_INTEGER_16"],
		8: ["MACH_MSG_TYPE_INTEGER_8",
			"MACH_MSG_TYPE_CHAR",
			"MACH_MSG_TYPE_BYTE"],
		1: ["MACH_MSG_TYPE_BIT"],
		0: ["MACH_MSG_TYPE_POLYMORPHIC",
			"MACH_MSG_TYPE_UNSTRUCTURED",
			"MACH_MSG_TYPE_REAL",
			"MACH_MSG_TYPE_STRING",
			"MACH_MSG_TYPE_STRING_C",
			"MACH_MSG_TYPE_PORT_internal",
			"MACH_MSG_TYPE_SEND_internal",
			"MACH_MSG_TYPE_RECEIVE_internal"],
	})
	
	def size_in_bits(self, typename):
		return self.TYPE_MAP[typename]
	

class ArmTypeSizes(BaseTypeSizes):
	# Just an example of what we could do with arch-specific information.
	pass

class Generic32BitTypeSizes(BaseTypeSizes):
	# As per BaseTypeSizes
	pass

class Generic64BitTypeSizes(BaseTypeSizes):
	TYPE_MAP = _expand({
		128: ['fpage'], # Struct with two longs
		80: ['long double'],
		64: ['signed long long int', 'unsigned long long int', 'double',
				'signed int', 'unsigned int', 'boolean', 'signed long int', 'unsigned long int',
				'signed __int64', 'unsigned __int64'],
		32: ['float'], # FIXME?
		16: ['signed short int', 'unsigned short int', 'wchar_t', 'wchar'],
		8: ['signed char', 'unsigned char', 'octet'],
		1: ['bool'], # FIXME?
		0: ['void', 'string', 'wstring', 'smallstring', '__builtin_va_list'],
	})

ARCHNAME_CLASS_MAP = {
	'Generic 32': Generic32BitTypeSizes,
	'Generic 64': Generic64BitTypeSizes
}
def construct_for_arch(arch_name, idl_type):
	if idl_type == 'mig':
		return MIGTypeSize()
	else:
		return ARCHNAME_CLASS_MAP[arch_name]()

