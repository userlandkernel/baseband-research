/*
 * @LICENCE("Open Kernel Labs, Inc.", "2007")@
 */

header {
# @LICENCE("Open Kernel Labs, Inc.", "2007")@
}

options {
    mangleLiteralPrefix = "TOKEN_";
    language=Python;
}

class rvct_ld_l extends Lexer;
options {
    k=5;
    exportVocab=RVCT_LD;
    charVocabulary= '\3'..'\377';
}

tokens {
           RO_CODE="RO-CODE";
           "CODE";
           RO_DATA="RO-DATA";
           "RO";
           "TEXT";
           RW_DATA="RW-DATA";
           "CONST";
           RW_CODE="RW-CODE";
           "RW";
           "DATA";
           "ZI";
           "BSS";
           "ENTRY";
           "FIRST";
           "LAST";
           "NOCOMPRESS";
           "ABSOLUTE";
           "PI";
           "RELOC";
           "OVERLAY";
           "EMPTY";
           ANY=".ANY";
}

WS :    (' '
   |    '\t'
   |    '\n'    { $newline; }
   |    '\r')
        { _ttype = SKIP; }
   ;


LPAREN
options {
    paraphrase="'('";
}
    :   '('
    ;

RPAREN
options {
    paraphrase="')'";
}
    :   ')'
    ;

LCURLY
    : '{'
    ;

RCURLY
    : '}'
    ;

PLUS
options{
    paraphrase="'+'";
}
    :   '+'
    ;

MINUS
options{
    paraphrase="'-'";
}
    :  '-'
    ;

COMMA
options{
    paraphrase="','";
}
    :   ','
    ;

protected
DIGIT
    :   '0'..'9'
    ;

INT :   (DIGIT)+
    ;

HEX
options {
  paraphrase = "a hexadecimal value value";
}

	:    ("0x" | "0X") ('0'..'9' | 'a'..'f' | 'A'..'F')+
	;


IDENT
options {
  paraphrase = "an identifer";
  testLiterals = true;
}

	:  ('*'|'a'..'z'|'A'..'Z'|'_'|'.') ('.'|'a'..'z'|'A'..'Z'|'_'|'0'..'9'|'*')*
	;

SL_PREPOCESSOR_DIRECTIVE :
    '#'
    (~'\n')* '\n'
    { _ttype = Token.SKIP; $newline; }
    ;

