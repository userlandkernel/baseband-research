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

import re, sys
import os, errno
import md5
from cStringIO import StringIO

from magpie.targets.shared.options import options
from magpie import helper

#NB: Dparser doesn't work with psyco for some reason. No doubt because dparser is unspeakably evil.
#try:
#	import psyco
#except ImportError:
#	pass
#else:
#	psyco.full()

def make_dirs_for_file(filename):
	filepath = os.path.split(filename)[0]
	try:
		os.makedirs(filepath)
	except OSError, e:
		if e.errno != errno.EEXIST:
			raise

def run(template_filename, template_locals, template_globals):
	full_path = os.path.join(options['path_to_package'],
			options['template_base'], template_filename)

	hash_func = md5.new()
	hash_func.update(file(full_path).read())
	template_hash = hash_func.hexdigest()
	
	cache_path = os.path.join(helper.get_magpie_dir('template_cache_dir', options),
			template_filename + template_hash)
	
	if not os.path.exists(cache_path):
		# Compile to cache!
		template_in = file(full_path, 'r').read()
		template_out = Compiler().compile_template(template_in)
		make_dirs_for_file(cache_path)
		file(cache_path, 'w').write(template_out)
	
	# Now run it...
	execfile(cache_path, template_globals, template_locals)
	
def markup(template_filename, output, template_globals):
	""" Main function to run a template.

	Template scope is isolated from the caller so this is
	equivalent to calling run() with a new locals dict.
	"""
	template_globals['_run'] = run
	template_globals['_output'] = output
	template_globals['_sys'] = sys
	run(template_filename, {}, template_globals)

def sstarts(s, pos, compare):
	return s[pos : pos + len(compare)] == compare

# Code to compile a template and return a string
# Usage:
# Compiler().compile_template(template) -> string
RUN_FN_NO_EXPLICIT_SCOPES = """def run(template_filename, **kwargs):
	caller = _sys._getframe(1)
	caller.f_locals.update(kwargs)
	_run(template_filename, caller.f_locals, caller.f_globals)
"""

RUN_FN_EXPLICIT_SCOPES = """def run(template_filename, **kwargs):
	caller = _sys._getframe(1)
	_run(template_filename, kwargs, caller.f_globals)
"""

class Compiler:
	"""
	This class compiles a template. The template format is:
	/*-python code-*/: execute "python code"
	/*-?python code-*/: evaluate "python code" and replace the whole thing with the result.
	/*-
	python code
	...
	-*/: execute "python code". 
	/*LOOP <somelist>*/ ... /*ENDLOOP*/ For LOOPITEM in <somelist> evaluate ...
	/*-if condition*/ template /*fi-*/: Evaluate "template" if eval(condition)
	/*-ABORT-*/: return immediately.
	"""		
	def __init__(self):
		# TEMPLATES_INDICATORS is a list of (starter, finisher, func) in whatever order
		# is most natural. There are no special requirements
		self.options = {'explicit_linebreaks': False,
				'isolated_scopes': False}

		TEMPLATES_INDICATORS = [ ('/*-', '-*/', self.t_code),
			('{*-', '-*}', self.t_code),
			('/*-if', '/*fi-*/', self.t_if),
			('{*-if', '{*fi-*}', self.t_if),
			('/*-?', '-*/', self.t_eval),
			('{*-?', '-*}', self.t_eval),
			('/*LOOP', '/*ENDLOOP*/', self.t_loop),
			('{*LOOP', '{*ENDLOOP*}', self.t_loop),
		]
		# MAGIC_SEPARATORS separate antecedent and consequent in "if", and
		# separate iterator and body in "loop"
		self.MAGIC_SEPARATORS = ('*/', '*}')
		# TEMPLATE_STARTERS is a list of (starter, func) sorted by length of starter.
		self.TEMPLATE_STARTERS = [(ti[0], ti[2]) for ti in TEMPLATES_INDICATORS]
		self.TEMPLATE_STARTERS.sort(lambda x, y: cmp(len(x[0]), len(y[0])))
		self.TEMPLATE_STARTERS.reverse()
		# TEMPLATE_FINISHERS is a list of (finisher, func) sorted by length of finisher
		self.TEMPLATE_FINISHERS = [(ti[1], ti[2]) for ti in TEMPLATES_INDICATORS]
		self.TEMPLATE_FINISHERS.sort(lambda x, y: cmp(len(x[0]), len(y[0])))
		self.TEMPLATE_FINISHERS.reverse()

		self.code_gen = CodeGen()

	def t_code(self, tmpl_piece):
		self.code_gen.output_code(tmpl_piece)

	def t_if(self, tmpl_piece):
		antecedent, consequent = self.split_closest(tmpl_piece, self.MAGIC_SEPARATORS)
		self.code_gen.output_if(antecedent)
		self._compile(consequent)

	def t_eval(self, tmpl_piece):
		self.code_gen.output_eval(tmpl_piece)

	def t_loop(self, tmpl_piece):
		arguments, body = self.split_closest(tmpl_piece, self.MAGIC_SEPARATORS)
		try:
			iterator_name, iterable = arguments.split('=', 1)
		except ValueError: # No iterator names supplied
			iterator_name = 'LOOPITEM'
			iterable = arguments
		self.code_gen.output_loop(iterator_name, iterable)
		self._compile(body)
	
	def split_closest(self, haystack, needle_list):
		pos = 0
		while pos < len(haystack):
			for needle in needle_list:
				if sstarts(haystack, pos, needle):
					return haystack.split(needle, 1)
			pos += 1
		raise Exception("No closest to split!")

	def find_same_level(self, haystack, indicator_func, startpos):
		stack = 1 # We are called *after* the starter.
		pos = startpos
		#print "fsl, startpos is", startpos, 'indicator func is', indicator_func
		while pos < len(haystack):
			for candidate_starter, candidate_func in self.TEMPLATE_STARTERS:
				if sstarts(haystack, pos, candidate_starter):
					if candidate_func == indicator_func:
						# New level down.
						stack += 1
			for candidate_finisher, candidate_func in self.TEMPLATE_FINISHERS:
				if sstarts(haystack, pos, candidate_finisher):
					if candidate_func == indicator_func:
						stack -= 1
						if stack == 0:
							#print "returning new pos of", pos
							return pos, len(candidate_finisher)
			pos += 1
		raise Exception("No end found (start was %s, pos %d-%d in template)" % (haystack[startpos:startpos+160], startpos, pos))
	
	def position_starts_template(self, haystack, pos):
		# return function, length of starter
		for candidate_starter, candidate_func in self.TEMPLATE_STARTERS:
			if sstarts(haystack, pos, candidate_starter):
				# Found one...
				return candidate_func, len(candidate_starter)
		return None, 0
	
	def compile_template(self, template):
		# Create a "run" function.
		self.code_gen.start()

		if self.options['isolated_scopes']:
			self.code_gen.output_code(RUN_FN_EXPLICIT_SCOPES)
		else:
			self.code_gen.output_code(RUN_FN_NO_EXPLICIT_SCOPES)

		self.code_gen.stop()

		# If the first line of the template is an options line, add them.
		if template.startswith('!!'):
			options_line, template = template.split('\n', 1)
			for option_string in options_line[2:].split(' '):
				self.options[option_string] = True
		return self._compile(template)

	def _compile(self, template):
		window_start = 0
		window_end = 0
		eof = len(template)

		self.code_gen.start()

		while window_start < eof:
			handler_func, skipstart = self.position_starts_template(template, window_end)
			while handler_func is None and window_end < eof:
				window_end += 1
				handler_func, skipstart = self.position_starts_template(template, window_end)
			# Now window_start points to just past the end of the previous template
			# and window_end points at the first character of the next template.
			#print "Test 1: window_start is", window_start, "window_end is", window_end
			if window_start != window_end:
				literal = template[window_start:window_end]
				if self.options['explicit_linebreaks'] is True:
					literal = re.sub('(?<!\\\\n)\n', '', literal)
					literal = literal.replace('\\n\n', '\n')
				if literal != '':
					self.code_gen.output_literal(literal)
			if window_end == eof:
				break # End of file
			window_start = window_end
			window_end, skipend = self.find_same_level(template, handler_func, window_end + 1)
			# Now window_start points to the beginning of the template and
			# window_end points to the first character marking the end of the template.
			#print "Test 2: window_start is", window_start, "window_end is", window_end
			handler_output = handler_func \
					(template[window_start + skipstart : window_end])
			window_start = window_end = window_end + skipend

		self.code_gen.stop()
		return self.code_gen.get()

class CodeGen:
	def __init__(self):
		self.clear()
	
	def clear(self):
		self.loc = []
		self.indent_level = -1
	
	def start(self):
		self.indent_level += 1
	
	def stop(self):
		self.indent_level -= 1
	
	def get(self):
		return '\n'.join(self.loc)

	def output_code(self, code):
		code_lines = code.split('\n')
		self._add_code_lines(code_lines)
	
	def output_if(self, antecedent):
		self._add_code('if %s:' % (antecedent))
	
	def output_eval(self, code):
		self._add_code("_output.write(str(%s))" % (code))
	
	def output_loop(self, iter_name, iterable):
		self._add_code('for %s in %s:' % (iter_name, iterable))
	
	def output_literal(self, data):
		self._add_code("_output.write(%s)" % (repr(data)))
	
	def _add_code(self, code_line):
		self.loc.append('%s%s' % ('\t' * self.indent_level, code_line))
	
	def _add_code_lines(self, code_lines):
		for line in code_lines:
			self._add_code(line)
