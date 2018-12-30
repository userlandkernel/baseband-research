import sys
from cStringIO import StringIO

from magpieparsers.antlr import antlr
sys.modules['antlr'] = antlr

import PrologParser, PrologLexer


def _parse(txt, func):
	handle = StringIO(txt)
	lexer = PrologLexer.Lexer(handle)
	parser = PrologParser.Parser(lexer)
	return getattr(parser, func)()

def parse_term(txt):
	return _parse(txt, 'topterm')

def parse_clauses(txt):
	return _parse(txt, 'clauses')

def parse(filename):
	handle = file(filename)
	lexer = PrologLexer.Lexer(handle)
	parser = PrologParser.Parser(lexer)
	result = parser.clauses()
	handle.close()
	return result

def test():
	QS = """
quicksort([], []).
quicksort([X], [X]).
quicksort([P|L], Z) :- partition(P, L, LEFT, RIGHT), 
					 quicksort(LEFT, R1), quicksort(RIGHT, R2),
					 append(R1, [P|R2], Z).

partition(_, [], _, _).
partition(P, [H|LIST], [H|L], R) :- H =< P, partition(P, LIST, L, R).
partition(P, [H|LIST], L, [H|R]) :- partition(P, LIST, L, R).
"""
	parse_clauses(QS).print_tree()

if __name__ == '__main__':
	test()

