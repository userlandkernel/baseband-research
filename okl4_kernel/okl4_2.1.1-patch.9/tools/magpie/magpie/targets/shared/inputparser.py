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

"""
I just read this and confused myself. :) The major source of confusion stemmed from the fact
that there are two ways in which cpp is run here: once to preprocess the IDL file, and once
to preprocess any C header files that are imported by the IDL file. - nfd 2007-07-29
"""

import re
import popen2
import os.path

from magpieparsers.cplusplus import parse_data as cparse
from magpieparsers.corba3 import parse_data as idlparser
from magpieparsers.mig.parser import parse as migidlparser
from magpieparsers.cpreprocessor.preprocess import cpp_file

from magpie.debugging import debug_import
from magpie.helper import usr_bin_which

class ASTImportError(Exception):
	def __init__(self, *args, **kwargs):
		Exception.__init__(self, *args, **kwargs)
		self.show_traceback = True

class FileNotFoundError(ASTImportError):
	pass

class CImportError(ASTImportError):
	pass

class CPPNotFoundError(CImportError):
	def __init__(self, *args, **kwargs):
		CImportError.__init__(self, *args, **kwargs)
		self.show_traceback = False

class _old_GenericASTInfo(object):
	def __init__(self, ast, **kwargs):
		self.ast = ast
		self.info = kwargs
	
	def __getitem__(self, key):
		return self.info[key]

# FIXME: This should be elsewhere probably
class CASTImporter(object):
	def __init__(self, filename_list, cpp = None, cpp_options = None,
			include_dirs = None, defines = None):

		self.filename_list = filename_list
		self.cpp = cpp or 'cpp'
		self.cpp_options = cpp_options or []
		self.include_dirs = include_dirs or []
		self.defines = defines or []

	def preprocess_and_grab_data(self, filename_list):
		# Make a tiny C file containing all the filenames as included.
		cpp_file = ['#include "%s"' % (filename) for filename in filename_list]
		cpp_file = '\n'.join(cpp_file) + '\n'
		# Ensure that the specified cpp actually exists
		full_cpp_path = usr_bin_which(self.cpp)
		if full_cpp_path is None:
			raise CPPNotFoundError("C pre-processor '%s' not found" % cpp)
		cpp_command_line = ' '.join ([self.cpp] + self.cpp_options \
			+ ['-I%s' % (incdir) for incdir in self.include_dirs]
			+ ['-D%s' % (define) for define in self.defines]
		)
		debug_import ("***** cpp: %s" % (cpp_command_line))
		child_out, child_in = popen2.popen2(cpp_command_line)
		child_in.write(cpp_file)
		child_in.close()
		return child_out.read()
		
	def run(self, baseast):
		debug_import ("***** Preprocessing %s" % (self.filename_list))
		data = self.preprocess_and_grab_data(self.filename_list)
		
		# Calculate the path to the parse tables.
		filename_for_parser = '_'.join(self.filename_list)
		try:
			#print '*** DATA'
			#print data
			#print 'END *** DATA'
			ast = cparse(baseast, filename_for_parser, data)
		except Exception, e:
			print "*** Exception encountered while processing %s" % (self.filename_list)
			file("inputparser.error.out", 'w').write(data)
			print "Preprocessed input written to inputparser.error.out"
			raise
		return ast
	
	def _dump_toplevel_types(self, ast):
		raise NotImplementedError()

class ASTGeneratorCImportMixIn(object):
	# Import handing as of sep 2005:
	# - We now scan the whole file prior to handing it to the parser, and perform
	#   all imports in one go. This is handled a bit poorly.
	def __init__(self, **kwargs):
		self.cpp_kwargs = kwargs

	def handle_import_statement(self, import_filename):
		# What to do when we get an import statement - basically
		# 1. Get the filename from the AST
		# 2. Run the C preprocessor over the AST, making sure to
		#    supply all the relevant include directories and flags
		# 3. Feed the result to the C parser and get an AST
		# 4. Run idltypes/castwalker over the ast to update the type
		#    registry.
		importer = CASTImporter(import_filenam)
		ast = importer.run()
		return ast

	def handle_import_statement_dummy(self, import_filename):
		pass
	
	def perform_imports(self, baseast, data):
		import_re = re.compile(r'^import +"(.*?)";', re.MULTILINE)
		filenames = import_re.findall(data)
		if filenames:
			importer = CASTImporter(filenames, **self.cpp_kwargs)
			ast = importer.run(baseast)
		
class GenericASTGenerator(object):
	def __init__(self, idl_cpp = None, cpp_options = None, include_dirs = None, defines = None):
		self._cpp_cmdline = {'cpp': idl_cpp or 'cpp',
				'options': cpp_options or '',
				'include_dirs': include_dirs or [],
				'defines': defines or [],
		}

	def produce_ast(self, idl_filename, filetype = 'CORBA', baseast = None):
		try:
			data = cpp_file(idl_filename, **self._cpp_cmdline)
		except IOError:
			raise FileNotFoundError(idl_filename)
		self.perform_imports(baseast, data) # FIXME

		if filetype == 'mig':
			raw_ast = migidlparser(idl_filename, data, baseast)
		else:
			baseast = idlparser(idl_filename, data, baseast)

		return baseast
		

class ImportingASTGenerator(GenericASTGenerator, ASTGeneratorCImportMixIn):
	def __init__(self, cpp = None, idl_cpp = None, cpp_options = None, include_dirs = None,
			defines = None):
		GenericASTGenerator.__init__(self, idl_cpp = idl_cpp, cpp_options = cpp_options,
				include_dirs = include_dirs, defines = defines)
		ASTGeneratorCImportMixIn.__init__(self, cpp = cpp, cpp_options = cpp_options,
				include_dirs = include_dirs, defines = defines)
	
	def produce_ast(self, idl_filename, filetype = 'CORBA', ast = None):
		return GenericASTGenerator.produce_ast(self, idl_filename, filetype, ast)
		
	def product_migast(self, filename):
		data = file(filename, 'r').read()
		self.perform_imports(data) # FIXME
		return data

def construct(cpp = None, idl_cpp = None, cpp_options = None, include_dirs = None, defines = None):
	return ImportingASTGenerator(cpp = cpp, idl_cpp = idl_cpp, cpp_options = cpp_options,
			include_dirs = include_dirs, defines = defines)

