"""
Specialised tester for IDL; we want:

- Display parse tree
- Display AST
"""

import sys, re
from optparse import OptionParser

from magpieparsers.corba3 import parse_to_pt, parse_to_ast
from magpieparsers.types.builtin import create_basictype_ast
from magpieparsers.parser_common import print_tree_pl

def main():
	# Parse command-line arguments and dispatch to appropriate tester.
	parser = OptionParser()
	#parser_inst.setup()
	parser.add_option('--pt', dest = "pt", action = 'store_true', default = False,
			help = 'Display the IDL parse tree')
	parser.add_option('--ast', dest = 'ast', action = 'store_true', default = False,
			help = 'Display the IDL AST')
	parser.add_option('--ast-pl', dest = 'ast_pl', action = 'store_true', default = False,
			help = 'Display the IDL AST in a Prolog-friendly format')
	parser.add_option('--types', dest = 'types', action = 'store_true', default = False,
			help = 'Import types and display the type registry')
	parser.add_option('--no-basic', dest = 'no_basic', action = 'store_true',
			default = False, help = 'Do not AST display basic types')
	cmdlineoptions, cmdlineargs = parser.parse_args()

	if cmdlineargs == []: # No files, so exit
		parser.print_help()
		print >>sys.stderr, "\nExiting because no IDL files were specified.\n"
		sys.exit(1)
	elif len(cmdlineargs) > 1: # Too many files!
		parser.print_help()
		print >>sys.stderr, "\nExiting because more than 1 IDL file was specified.\n"
		sys.exit(1)
		
	#if cmdlineoptions.ugly_includepath_hack is not None:
	#	cmdlineoptions.include_dirs = [os.path.join(cmdlineoptions.ugly_includepath_hack, i)
	#		for i in cmdlineoptions.include_dirs]

	hardware_arch = 'Generic 32'
	generator_type = 'idl4'
		
	pt = parse_to_pt(cmdlineargs[0])
	
	if cmdlineoptions.pt:
		print "*** PT:"
		pt.print_tree()
		print "*** PT ends"
	
	if cmdlineoptions.ast or cmdlineoptions.ast_pl:
		basicast = create_basictype_ast(hardware_arch, generator_type)
		if cmdlineoptions.no_basic:
			for child in basicast.children:
				child.nonprinting = True
			name = 'AST (no basic types)'
		else:
			name = 'AST'
		
		basicast = parse_to_ast(pt, cmdlineargs, basicast)
		if cmdlineoptions.ast:
			print "*** %s:" % (name)
			basicast.print_tree(hide_hidden = True)
			print "*** AST ends"
		else:
			assert cmdlineoptions.ast_pl
			print_tree_pl(basicast)
	
if __name__ == '__main__':
	main()
