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

class gnu_ld_l extends Lexer;
options {
    k=5;
    exportVocab=GNU_LD;
    charVocabulary= '\3'..'\377';
}

tokens {
           "SECTIONS";
           "PHDRS";
           "MEMORY";
           "OVERLAY";
           "BLOCK";
           "ENTRY";
           "PROVIDE";
           "PROVIDE_HIDDEN";
           "AT";

           /* BUILTIN functions */
           "ABSOLUTE";
           "ADDR";
           "LOADADDR";
           "ALIGN";
           "DEFINED";
           "NEXT";
           "SIZEOF";
           "MAX";
           "MIN";
           "CONSTANT";

           /* BUILTIN commands */
           "OUTPUT_FORMAT";
           "ASSERT";
           "OUTPUT_ARCH";
           "SEARCH_DIR";
           "INPUT";
           "OUTPUT";
           "GROUP";
           "STARTUP";
           "TARGET";
           "NOCROSSREFS";
           "DATA_SEGMENT_ALIGN";
           "DATA_SEGMENT_END";
           "DATA_SEGMENT_RELRO_END";
           "BYTE";
           "SHORT";
           "LONG";
           "QUAD";
           "SQUAD";
           "FILL";

}

WS :    (' '
   |    '\t'
   |    '\n'    { $newline; }
   |    '\r')
        { _ttype = SKIP; }
   ;

ML_COMMENT
  : "/*"
    (               /* '\r' '\n' can be matched in one alternative or by matching
                       '\r' in one iteration and '\n' in another. I am trying to
                       handle any flavor of newline that comes in, but the language
                       that allows both "\r\n" and "\r" and "\n" to all be valid
                       newline is ambiguous. Consequently, the resulting grammar
                       must be ambiguous. I'm shutting this warning off.
                    */
      options {
        generateAmbigWarnings=false;
      }
      :  { self.LA(2)!='/' }? '*'
      | '\r' '\n' {self.newline();}
      | '\r' {self.newline();}
      | '\n' {self.newline();}
      | ~('*'|'\n'|'\r')
    )*
    "*/"
    {$setType(Token.SKIP);}
;


DISCARD : "/DISCARD/";
NOLOAD : "(NOLOAD)";
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
options {
    paraphrase="'{'";
}
    : '{'
    ;

RCURLY
options {
    paraphrase="'}'";
}
    : '}'
    ;

COMMA
options{
    paraphrase="','";
}
    :   ','
    ;


SEMICOLON
options{
    paraphrase="';'";
}
    : ';'
    ;

COLON
options{
    paraphrase="':'";
}
    : ':'
    ;

QMARK
options{
    paraphrase="'?'";
}
    : '?'
    ;

EQUALS
options{
    paraphrase="'='";
}
    : {self.LA(2) != '='}? '=' 
    ;

OP_STAR : {self.LA(2) != '='}? '*';
OP_MINUS : {self.LA(2) != '='}? '-';

OP_OTHERS: {self.LA(2) != '='}? ('+' | '/' | '%' );
OP_AND: {self.LA(2) != '='}? '&' ('&')?;
OP_OR: {self.LA(2) != '='}? '|' ('|')?;

protected
OP_EQ : ('!'|'~'|'<'|'>') ('=')?;

OPERATOR : OP_EQ | "<<" | ">>" | "==";
OP_ASSIGN: "*=" | "+=" | "/=" | "-=" | "&=" | "|=" | "%=";



protected
DIGIT
    :   '0'..'9'
    ;

protected 
IDENT_FIRSTCHAR
    : ( 'a'..'z' | 'A'..'Z' | '_' | '.' )
    ;

protected
IDENT_CHAR
    : (IDENT_FIRSTCHAR | DIGIT | '*' | '-' | '?')
    ;

IDENT
options {
  paraphrase = "an identifer";
  testLiterals = true;
}
	: (('"' (~('"'))* '"') /*" fix quoting for vim */
        | (IDENT_FIRSTCHAR (IDENT_CHAR)*))
        ;

INT :   (DIGIT)+ ('K' | 'M' | 'k' | 'm')?
    ;

/*OCTAL 
options {
    paraphrase = "an octal value";
}
    :   ('0') ('0'..'7')+
    ;
*/
HEX
options {
  paraphrase = "a hexadecimal value";
}

	:    ("0x" | "0X") ('0'..'9' | 'a'..'f' | 'A'..'F')+
	;




