"""
C++ parser tests.
"""
import unittest
import cStringIO
import traceback
import os

from snakebag.testmenu import TestMenu

from magpieparsers.cplusplus import parse_to_pt, parse_to_ast
from magpieparsers.error import Error as ParserError
from magpieparsers.types.builtin import create_basictype_ast

class CPPTestMixIn(object):
	# FIXME: Cut-and-pasted with mods from test_ast.py
	def _parse(self, filename, output, options = None):
		if options is None:
			options = OPTIONS
	
		try:
			pt = parse_to_pt(filename, accelerate = self.ACCELERATE)
		except ParserError, e:
			print >>output, "* Parser error *"
			print >>output, str(e)
		else:
			return self._parse_pt_to_ast(pt, filename, data, output, options)
	
	def _parse_pt_to_ast(self, pt, filename, data, output, options = None):
		basicast = create_basictype_ast(options['hardware_arch'], options['generator'])
		for child in basicast.children:
			child.nonprinting = True
		try:
			basicast = parse_to_ast(pt, filename, basicast)
		except ParserError, e:
			print >>output, "* Semantic error *"
			print >>output, str(e)
		except:
			print >>output, "* ERROR *"
			traceback.print_exc(None, output)
		else:
			basicast.print_tree(handle = output, hide_hidden = True)
	
	def _do(self, filename, options = None):
		input_filename = os.path.join(self.INPUT_DIR, filename)

		handle = cStringIO.StringIO()
		self._parse(input_filename, handle, options)
		new_output = handle.getvalue()

		output_filename = os.path.join(self.OUTPUT_DIR, filename)
		
		menu = TestMenu()
		menu.go(data, output_filename, new_output)
	
class CPPInCPPTest(unittest.TestCase, CPPTestMixIn):
	ACCELERATE = True
	INPUT_DIR = os.path.join('test', 'input', 'cplusplus')
	OUTPUT_DIR = os.path.join('test', 'output', 'cplusplus')

	def test_simple(self):
		pass


