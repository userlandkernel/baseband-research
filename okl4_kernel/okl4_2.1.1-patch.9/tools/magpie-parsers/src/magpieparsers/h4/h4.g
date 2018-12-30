options {
	language = "Python";
}

class H4Parser extends Parser;

options {
	k = 2;
	buildAST = true;
}

declarations : (declaration)*
	;

declaration : func_decl
	| module_decl
	;

module_decl : "module"! IDENT LCURLY! declarations RCURLY!
	;

func_decl : func_name DEFINED_AS! expression SEMI!
	;

func_name: IDENT
	| (IDENT COLON (literal | constructed_type | IDENT) )+ (IDENT)?
	;

expression : add_expr
	;

add_expr : mult_expr ( (PLUS | MINUS) mult_expr)*
	;

mult_expr : unary_expr ( (STAR | DIV | MOD) unary_expr)*
	;

unary_expr : (MINUS | PLUS | TILDE) primary_expr
	| primary_expr
	;

primary_expr :
	literal
	| constructed_type
	| scoped_name
	| LBRACKET expression RBRACKET
	;

constructed_type : node_constructor
	;

node_constructor : HASH! expression HASH!
	;

scope : IDENT PERIOD
	;

scoped_name : (scope)? func_name
	;

literal : integer_literal
	| atom_literal
	| string_literal
	;

integer_literal : INT
	;

atom_literal : ATOM
	;

string_literal : QUOTEDSTRING
	;

class H4Lexer extends Lexer;

options {
	k = 2;
}

DEFINED_AS  : ":-" ;
PERIOD : '.' ;
LBRACKET : '(' ;
RBRACKET : ')' ;
LCURLY : '{' ;
RCURLY : '}' ;
COMMA : ',' ;
COLON : ':' ;
PLUS : '+' ;
MINUS : '-' ;
STAR : '*' ;
DIV : '/' ;
MOD : '%' ;
TILDE : '~' ;
HASH : '#' ;
SEMI : ';' ;

ATOM : ("'" (LETTER | DIGIT | '_')+ ) ;
IDENT : (LETTER | '_')  (LETTER | DIGIT | '_')* ;
INT : (DIGIT)+ ;
protected LETTER : ('A'..'Z') | ('a' .. 'z') ;
protected NONZERODIGIT : '1'..'9' ;
protected DIGIT : '0' .. '9' ;
QUOTEDSTRING : ('"' (LETTER | '-' | DIGIT | '.' | '_' | '/')+ '"') ;
WS    : ( ' ' | EndOfLine | '\t')
        {$setType(Token.SKIP);}
      ;
Comment
        :       "//" (~('\n' | '\r'))* EndOfLine
                {$skip;}                     
        ;
protected EndOfLine
	:       (       options{generateAmbigWarnings = false;}:
			"\r\n"  {$newline;} // MS
		|       '\r' {$newline;}   // Mac
		|       '\n' {$newline; }    // Unix
		)
	;

