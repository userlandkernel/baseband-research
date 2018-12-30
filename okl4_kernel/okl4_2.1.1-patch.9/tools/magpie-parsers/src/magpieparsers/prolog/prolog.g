// Created by Nicholas FitzRoy-Dale (wzdd@lardcave.net) 2006-09-24
// Public domain code. Email me if you need clarification.

header {
from magpieparsers.parser_common import Node

def expr_node(parent, oper, child):
    container = Node(None, "expression", leaf = oper, children = [parent, child])
    return container
}

header "PrologParser.__init__" {
self.terms = []
}

options {
    language  = "Python";
}

class PrologParser extends Parser;
options {
	k = 2;
	exportVocab=Prolog;
}

/* Make an explicit start rule so ANTLR matches EOF after 'term'. */
topterm returns[t] : t = term ;

clauses returns[l]
	{l = Node(None, "clauses")}
	: c = clause {l.add_child(c)} (c = clause {l.add_child(c)})*
	;

clause returns[c]
	{c = Node(None, "clause")}
	:
	( s = structure {c.add_child(s)}
	 | a = atom{c.add_child(a)}
	)?
	
	(DEFINEDAS e=expression {c.add_child(e)} )?  FULLSTOP
	;

/* Precedence from http://cs.wwc.edu/~aabyan/Logic/Prolog.html */
/* No maths support yet so a lot of it is unused.
 * I don't expect this Prolog will support custom operators for quite a while. */
expression returns[e] : o = or_expr {e = o} (SEMICOLON {op = ";"} o2 = or_expr {e = expr_node(e, op, o2)} )*;

or_expr returns[e] : o = not_expr {e = o} (COMMA o2= not_expr {e = expr_node(e, ",", o2)} )*;

not_expr returns[e] : {hitnot = False} (NOT {hitnot = True})? e = relation_expr {if hitnot: e = Node(None, "expression", leaf = "not", children = [e])} ;

relation_expr returns[e]:
	e = basic_expr
	(o = relation_op e2 = basic_expr
	 {e = Node(None, "expression", leaf = o, children=[e, e2])}
	)*
	;

relation_op returns[o]:
	LESSTHANOREQ {o = "lessthanorequals"}
	| GREATERTHANOREQ {o = "greaterthanorequals"}
	| GREATERTHAN {o = "greaterthan"}
	| LESSTHAN {o = "lessthan"}
	| EQUAL {o = "unify"}
	;

/* add_expr returns [e]: unary_expr (PLUS | MINUS unary_expr)*; */

/* unary_expr returns[e]: (PLUS | MINUS)? mult_expr; */

/* mult_expr returns[e]: basic_expr (STAR | SLASH) basic_expr ;
*/

basic_expr returns[e]: t = term {e = t}
		| LBRACKET o = expression {e = o} RBRACKET
		;

term returns[t]
	: a = atom {t = a}
	| v = variable {t = v}
	| l = prologlist {t = l}
	| s = structure {t = s}
	| n = number {t = n}
	| d = dictionary {t = d}
	;

atom returns[t]
	: a:ATOM {t = Node(None, "atom", leaf = a.getText())}
		{if t.leaf.startswith("'"): t.leaf = t.leaf[1:-1]}
	;

variable returns[t]
	: v:VARIABLE {t = Node(None, "variable", leaf = v.getText())}
	;

number returns[n]
	: nt:NUMBER {n = Node(None, "number", leaf = nt.getText())}
	;

/* Python-like dictionary -- currently only empty dicts supported by the parser */
dictionary returns[d]
	: LCURLY RCURLY
	{d = Node(None, "dictionary"); d._internal_data = {}}
	;

/* Prolog lists. Can be empty, can be a set length, or can consist of a head
 * and a tail separated by the list constructor '|'.
 * Examples: [], [a], [Var, blah, 37], [H|T], [x, y, Pirates|Tail]
 * We automatically convert them to the structure form .(H, T)
 * here.
 * [a, b, c | D] -> .(a, .(b, .(c, D)))
*/
prologlist returns[l]
	: LSQUARE RSQUARE {l = Node(None, "structure", leaf = '.')}
	| LSQUARE c = list_contents RSQUARE {l = c}
	;

/* Separate rules for list and list_contents because a nonempty list
 * must always contain a head element (ie not just a tail element)
*/
list_contents returns[l]
{
def add_listterm(t, parent):
    child = Node(None, "structure", leaf = '.', children = [t])
    parent.add_child(child)
    return child
}
	: t = term 
		{l = Node(None, "structure", leaf = '.', children = [t]); subnode = l}
		(COMMA t=term {subnode = add_listterm(t, subnode)})*
	  (BAR (t2 = variable | t2 = prologlist) {subnode.add_child(t2)})?

	  /* If the resulting list has only one element (the head) the tail is
	   * implicitly the empty list. 
	  */
{
if len(subnode.children) == 1:
    subnode.add_child(Node(None, "structure", leaf = '.'))
}
	;

structure returns[s]: 
	{s = Node(None, "structure")}
	f:ATOM  {s.leaf = f.getText()}
	LBRACKET (t = termlist {s.children.extend(t)})?  RBRACKET
	;

termlist returns [tl]: t = term {tl = [t]} (COMMA t = term {tl.append(t)})* ;

class PrologLexer extends Lexer;
options {
   k = 2;
   exportVocab=Prolog; /* Nice try? */
}

WS : (' ' | '\t' | '\n' {$nl;})
	{$skip};

COMMENT : '%' ( ~('\n') )*
	{$skip};

SEMICOLON : ';';
BAR : '|';
LSQUARE : '[';
RSQUARE : ']';
LBRACKET : '(' ;
RBRACKET : ')' ;
LCURLY : '{';
RCURLY : '}';
COMMA : ',' ;
FULLSTOP : '.' ;
DEFINEDAS : ":-" ;
LESSTHANOREQ : "=<";
GREATERTHANOREQ : "=>";
LESSTHAN : "<";
GREATERTHAN : ">";
EQUAL : "=";
NOT : "\\+";

NUMBER : ('0' .. '9')+ ;

/* Variable can be the anonymous variable '_' or any alphanum sequence starting
 * with a capital letter.
*/
VARIABLE : ('_')
		| (('A' .. 'Z') ('a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_')*) ;

/* Atoms must start with a lowercase letter or be enclosed in single quotes. */
ATOM : (('a' .. 'z') ('a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_')*)
		| ("'" ( ~'\'')+ "'")
	;

