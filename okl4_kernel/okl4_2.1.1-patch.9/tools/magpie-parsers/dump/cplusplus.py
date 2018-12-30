"""
Specialised tester for C++ import; we want:

- Display parse tree
- Display AST
- Import types and display registry.
"""
import sys, popen2
import cPickle
from optparse import OptionParser

# Use the local one, not the installed one.
sys.path.insert(0, "src")

from magpieparsers.cplusplus import parse_to_pt, parse_to_ast
from magpieparsers.types.builtin import create_basictype_ast

def main():
	# Parse command-line arguments and dispatch to appropriate tester.
	parser = OptionParser()
	parser.add_option('--cpp', dest = 'use_cpp', action = 'store_true', default = False,
			help = 'Preprocess the source file')
	parser.add_option('--pt', dest = 'pt', action = 'store_true', default = False,
			help = 'Display the C++ parse tree')
	parser.add_option('--ast', dest = 'ast', action = 'store_true', default = False,
			help = 'Display the C++ AST')
	parser.add_option('--types', dest = 'types', action = 'store_true', default = False,
			help = 'Import types and display the type registry')
	parser.add_option('--profile', dest = 'profile', action = 'store_true', default = False,
			help = 'Perform several parses and produce a report')
	parser.add_option('--magpie', dest = 'magpie_mode', action = 'store_true', default = False,
			help = 'Only test type generation (simulates magpie.py use)')
	parser.add_option('--pickle-pt', dest = 'pickle_pt', action = 'store', default = None,
			help = 'Pickle parse tree to named file')
	parser.add_option('--slow-parser', dest = 'slow_parser', action = 'store_true', default = False,
			help = 'Use the Python C++ parser (slow but does not require C extension)')
	my_options, args = parser.parse_args()

	if len(args) != 1:
		print >>sys.stderr, "Please specify one C file for processing.\n"
		sys.exit(1)

	filename = args[0]
	
	data = file(filename).read()

	if my_options.profile:
		loops = 5
	else:
		loops = 1
	
	accelerate = True
	if my_options.slow_parser:
		accelerate = False

	if my_options.pickle_pt and accelerate:
		print "*** Disabling acceleration for pickling"
		accelerate = False

	for count in range(loops):
		pt = parse_to_pt(filename, accelerate = accelerate)

	if my_options.pt:
		print "*** Parse tree:"
		pt.print_tree()
		print "*** Parse tree ends"
	
	if my_options.pickle_pt is not None:
		handle = file(my_options.pickle_pt, 'wb')
		cPickle.dump(pt, handle)
		handle.close()
		print "*** Wrote pickle to %s" % (my_options.pickle_pt)
	
	hardware_arch = 'Generic 32'
	generator_type = 'idl4'

	if my_options.ast:
		basicast = create_basictype_ast(hardware_arch, generator_type)
		if my_options.magpie_mode:
			types_only = True
			experimental = False
		else:
			experimental = True
			types_only = False

		ast = parse_to_ast(basicast, pt, filename, types_only = types_only,
				experimental = experimental)
		print "*** AST:"
		ast.print_tree()
		print "*** AST ends"

	if my_options.types and ast:
		#registry = gimme_a_registry()
		#walker = CASTWalker(registry)
		#walker.find_types(ast)

	
		print "*** Types list:"
		#registry.print_registry()
		print "*** Types list ends"
	
if __name__ == '__main__':
	main()
