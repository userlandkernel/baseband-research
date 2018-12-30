"""
Dump a Prolog AST (we don't have a parse tree)
"""

import sys, os
from optparse import OptionParser

# Use the local one, not the installed one.
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'src', 'magpieparsers'))
from antlr import antlr
from prolog import parse

def main():
	parser = OptionParser()
	options, args = parser.parse_args()

	filename = args[0]
	result = parse(filename)

	result.print_tree()

if __name__ == '__main__':
	main()

