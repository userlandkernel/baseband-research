import unittest
import sys
import os
import optparse

if os.getcwd() not in sys.path:
	sys.path.insert(0, os.getcwd())

from test_ast import IDLASTTest
from test_ast import IDLASTMICOTest
from test_cpp import CPPInCPPTest

OPTIONS = {
	'hardware_arch': 'Generic 32',
	'generator': 'idl4'
}

def main():
	parser = optparse.OptionParser()
	parser.add_option('--c3ast', dest = 'test_c3ast',
			action = 'store_true', default = False,
			help = 'Test suite: CORBA 3 AST')
	parser.add_option('--c3astmico', dest = 'test_c3ast_mico',
			action = 'store_true', default = False,
			help = 'Test suite: CORBA 3 AST: MICO')
	parser.add_option('--cpp', dest = 'test_cplusplus',
			action = 'store_true', default = False,
			help = 'Test suite: C++ (C++ parser)'),
	parser.add_option('--noninteractive', dest = 'noninteractive',
			action = 'store_true', default = False)
	options, args = parser.parse_args()
	
	suite = unittest.TestSuite()
	
	if options.test_c3ast:
		suite.addTest(unittest.makeSuite(IDLASTTest))
	if options.test_c3ast_mico:
		suite.addTest(unittest.makeSuite(IDLASTMICOTest))
	if options.test_cplusplus:
		suite.addTest(unittest.makeSuite(CPPInCPPTest))

	results = unittest.TestResult()
	suite.run(results)
	for error in results.errors:
		print '\n'.join(error[1:])
	print "Errors:", len(results.errors)
	print "Failures:", len(results.failures)

if __name__ == '__main__':
	main()
