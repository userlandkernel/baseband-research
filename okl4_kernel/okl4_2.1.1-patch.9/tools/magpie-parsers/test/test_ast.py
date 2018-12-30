"""
CORBA 3 parser tests
"""
import unittest
import cStringIO
import traceback
import os

from snakebag.testmenu import TestMenu

from magpieparsers.corba3 import parse_data_to_pt, parse_to_ast
from magpieparsers.error import Error as ParserError
from magpieparsers.types.builtin import create_basictype_ast

class ASTTestMixIn(object):
	def _parse(self, filename, data, output, options = None):
		if options is None:
			options = OPTIONS
	
		try:
			pt = parse_data_to_pt(filename, data)
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
		data = file(input_filename).read()

		handle = cStringIO.StringIO()
		self._parse(input_filename, data, handle, options)
		new_output = handle.getvalue()

		output_filename = os.path.join(self.OUTPUT_DIR, filename)
		
		menu = TestMenu()
		menu.go(data, output_filename, new_output)
	
class IDLASTTest(unittest.TestCase, ASTTestMixIn):
	INPUT_DIR = os.path.join('test', 'input', 'idlast')
	OUTPUT_DIR = os.path.join('test', 'output', 'idlast')

	def test_simple(self):
		# Self-check
		self._do('simple.idl')
	
	def test_forward_decl(self):
		# Issue #1044 test 2A
		self._do('forward_decl.idl')
	
	def test_explicit_scoping(self):
		# Issue #1044 test 2B
		self._do('explicit_scoping.idl')

	def test_explicit_scoping_2(self):
		# Issue #1044 test 2C
		self._do('explicit_scoping_2.idl')

	def test_reopened_scope(self):
		# Issue #1044 test 2D
		self._do('reopened_scope.idl')

	def test_reopened_scope_2(self):
		# Issue #1044 test 2E
		self._do('reopened_scope_2.idl')
	
	def test_interface_self_ref(self):
		# Issue #1044 test 2F
		self._do('interface_self_ref.idl')

	def test_explicit_scoping_3(self):
		# Issue #1044 test 2G
		self._do('explicit_scoping_3.idl')
	
	def test_wide_string_const(self):
		# Issue #1044 test 2H
		self._do('wide_string_const.idl')

	def test_expressions(self):
		# Issue #1044 test 2I
		self._do('expressions.idl')
	
	def test_case(self):
		# Issue #1044 test 2J
		self._do('case.idl')

	def test_octet_constant(self):
		# Issue #1044 test 2K
		self._do('octet_constant.idl')
	
	def test_enum_switch(self):
		# Issue #1044 test 2L
		self._do('enum.idl')
	
	def test_boolean_constants(self):
		# Issue #1044 test 2M
		self._do('boolean_constants.idl')
	
	def test_bad_characters(self):
		# Issue #1044 test 3A
		self._do('bad_characters.idl')
	
	def test_unknown_type(self):
		# Issue #1044 test 3B
		self._do('unknown_type.idl')
	
	def test_unknown_type_in_union(self):
		# Issue #1044 test 3C
		self._do('unknown_type_in_union.idl')
	
	#def test_bad_include(self):
	#	# Issue #1044 test 4A
	#	# FIXME: Doesn't work (though CPP does give an error)
	#	self._do('bad_include.idl')

	#def test_unbalanced_cpp_directives(self):
	#	# Issue #1044 test 4B
	#	# FIXME: Doesn't work (though CPP gives an error)
	#	self._do('unbalanced_cpp_directives.idl')

	def test_name_clash(self):
		# Issue #1044 test 4C
		self._do('name_clash.idl')
	
	def test_enumerator_clash(self):
		# Issue #1044 test 4D
		self._do('enumerator_clash.idl')
	
	def test_scoping(self):
		# Issue #1044 test 4F
		self._do('scoping.idl')
	
#	def test_duplicate_union_labels(self):
#		# Issue #1044 test 4G
#		# The issue this tests for is that there are no duplicated expressions in a Union.
#		# In CORBA this can't be decided in general at compile time because unions may
#		# include variables. However we can check constants. 
#		# FIXME: We currently don't do this
#		self._do('duplicate_union_labels.idl')

	def test_typedef_enum_clash(self):
		# Issue #1044 test 4H
		self._do('typedef_enum_clash.idl')
	
	def test_inheritance_1(self):
		# Issue #1044 test 4I
		# FIXME: Don't do this yet.
		# FIXME: Verify this one.
		self._do('inheritance_1.idl')

	def test_inheritance_2(self):
		# Issue #1044 test 4J
		self._do('inheritance_2.idl')
	
#	def test_instantiate_incomplete(self):
#		# Issue #1044 test 4K
#		# FIXME: Not sure about the justification for this one.
#		self._do('instantiate_incomplete_type.idl')
	
	def test_invalid_union_discriminator(self):
		# Issue #1044 test 4L
		self._do('union_invalid_discriminator.idl')
	
	def test_unknown_const(self):
		# Issue #1044 test 4M
		self._do('unknown_const.idl')
	
	def test_const_assignment_typecheck(self):
		# Issue #1044 test 4N
		# FIXME: Type checking not implemented yet
		self._do('const_assignment_typecheck.idl')

#	def test_union_case_typecheck(self):
#		# Issue #1044 test 4O
#		# FIXME: Type checking not implemented yet
#		self._do('union_typecheck.idl')

	def test_int_range_check(self):
		# Issue #1044 test 4P
		# FIXME: Type checking not implemented yet
		self._do('int_range_check.idl')

#	def test_float_range_check(self):
#		# Issue #1044 test 4Q
#		# FIXME: Float type checking not implemented yet
#		self._do('float_range_check.idl')

	def test_inheritance_3(self):
		# Issue #1044 test 4R
		# Can't inherit from the same class twice directly
		self._do('inheritance_3.idl')
	
	def test_void_parameter(self):
		# Issue #1044 test 4S
		# FIXME: "Magpie allows 'void' in a variety of invalid places, including attributes, parameters, sequences, and structs." (not tested)
		self._do('void_parameter.idl')
	
#	def test_invalid_float_operators(self):
#		# Issue #1044 test 4T
#		# FIXME: Type checking not implemented yet
#		self._do('invalid_float_operators.idl')

	def test_invalid_mixed_literals(self):
		# Issue #1044 test 4U
		# FIXME: Type checking not implemented yet
		self._do('mixed_literal_types.idl')
	
#	def test_shift_amount_validity(self):
#		# Issue #1044 test 4V
#		# FIXME: Type checking not implemented yet
#		self._do('invalid_shifts.idl')

	def test_negative_array_size(self):
		# Issue #1044 test 4W
		# FIXME: Type checking not implemented yet
		self._do('array_size.idl')

#	def test_potential_scope(self):
#		# Issue #1044 test 4X
#		# FIXME: Unsupported so far
#		self._do('potential_scope.idl')

	def test_type_redefinition(self):
		# Issue #1044 test 4Y
		self._do('typedef_redefinition.idl')

# Parser error-handling tests
	def test_reserved_word(self):
		# Issue #1044 test 5A
		self._do('reserved_word.idl')
	
	def test_empty(self):
		# Issue #1044 test 5B
		self._do('empty.idl')
	
	def test_mismatched_parentheses(self):
		# Issue #1044 test 5C
		self._do('mismatched_parentheses.idl')

	def test_missing_semicolon(self):
		# Issue #1044 test 5D
		self._do('missing_semicolon.idl')
	
	def test_string_as_discriminator(self):
		# Issue #1044 test 5E
		self._do('string_as_discriminator.idl')
	
	def test_missing_struct_member_name(self):
		# Issue #1044 test 5F
		self._do('missing_struct_member_name.idl')

	def test_typedef_multiple_decls(self):
		self._do('typedef_multiple_decls.idl')

	def test_union_as_function_param(self):
		self._do('union_as_function_param.idl')
	
	def test_wchar_literal(self):
		self._do('wchar_literal.idl')
	
	# Misc tests
	def test_string_constants(self):
		self._do('string_constants.idl')
	
	def test_integer_constants(self):
		self._do('integer_constants.idl')

	def test_char_constants(self):
		self._do('char_constants.idl')
	
	def test_general_types(self):
		self._do('idl_types.idl')
	
	def test_general_arrays(self):
		self._do('idl_arrays.idl')
	
	def test_duplicate_forward_decl(self):
		self._do('duplicate_forward_decl.idl')
	
	def test_enum_return(self):
		self._do('enum_return.idl')
	
	def test_unary_minus(self):
		self._do('unary_minus.idl')
	
	def test_pragmas(self):
		self._do('pragmas.idl')
	
	def test_string_param_1(self):
		self._do('string_param.idl')

	def test_string_param_2(self):
		self._do('string_param_2.idl')

	def test_scoped_param(self):
		self._do('scoped_param.idl')
	
	def test_scoped_return(self):
		self._do('scoped_type_return.idl')
	
	def test_aliased_string_const(self):
		self._do('aliased_string_const.idl')
	
	def test_embedded_struct(self):
		self._do('embedded_struct.idl')
	
	def test_struct_param(self):
		self._do('struct_param.idl')
	
	def test_union_embedded_enum(self):
		self._do('union_embedded_enum.idl')

class IDLASTMICOTest(unittest.TestCase, ASTTestMixIn):
	INPUT_DIR = os.path.join('../', 'magpie-tests', 'mico-idl', 'input')
	OUTPUT_DIR = os.path.join('../', 'magpie-tests', 'mico-idl', 'expected_output')

	def test_1(self):
		self._do('1-calc.idl')

	def test_2(self):
		self._do('2-calc.idl')
	
	def test_3(self):
		self._do('3-calc.idl')
	
	def test_4(self):
		self._do('4-calc.idl')

	def test_5(self):
		self._do('5-strtest.idl')
