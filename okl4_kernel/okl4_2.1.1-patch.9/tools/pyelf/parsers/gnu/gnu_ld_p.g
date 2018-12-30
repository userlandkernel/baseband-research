/*
 * @LICENCE("Open Kernel Labs, Inc.", "2007")@
 */

header {
# @LICENCE("Open Kernel Labs, Inc.", "2007")@
}

header "gnu_ld_p.__main__" {
    import gnu_ld_l
    import gnu_ld_p

    L = gnu_ld_l.Lexer()
    P = gnu_ld_p.Parser(L)
    P.setFilename(L.getFilename())

    // Parse the input expression
    try:
        P.script()
    except antlr.RecognitionException, ex:
        print "ERROR\n"

    ast = P.getAST()

    if not ast:
        print "stop - no AST generated."
        sys.exit(0)

    print "NB this script should only be run directly for debugging purposes"
    print "segment names seen in the gnu linker or \"scatter gather\" script provided on stdin are:"
    print getattr(P, "segment_names", [])
    print getattr(P, "section_names", [])
}

options {
    mangleLiteralPrefix = "TOKEN_";
    language=Python;
}

class gnu_ld_p extends Parser;
options {
    k=10;
    importVocab=GNU_LD;
    buildAST =true;
}

/* We ignore linker version scripts for the time being */

/* identifier hacks */
/* we need to accept * as both an identifier and an operator 
   depending upon it's context */
ident: (IDENT) | (OP_STAR);
func: TOKEN_ASSERT | TOKEN_ENTRY | TOKEN_ABSOLUTE | TOKEN_ADDR | TOKEN_LOADADDR | TOKEN_ALIGN
    | TOKEN_DEFINED | TOKEN_NEXT | TOKEN_SIZEOF | TOKEN_MAX | TOKEN_MIN | TOKEN_CONSTANT
    | TOKEN_OUTPUT_FORMAT | TOKEN_OUTPUT_ARCH | TOKEN_SEARCH_DIR | TOKEN_INPUT
    | TOKEN_OUTPUT | TOKEN_GROUP | TOKEN_STARTUP | TOKEN_TARGET | TOKEN_NOCROSSREFS
    | TOKEN_DATA_SEGMENT_ALIGN | TOKEN_DATA_SEGMENT_END | TOKEN_DATA_SEGMENT_RELRO_END
    | TOKEN_BYTE | TOKEN_SHORT | TOKEN_LONG | TOKEN_QUAD | TOKEN_SQUAD | TOKEN_FILL;

/* assignments and expressions */
/* we don't care about these so very simple parsing */
op: OPERATOR | OP_MINUS | OP_OTHERS | OP_AND | OP_OR | OP_STAR;
assign_op: EQUALS | OP_ASSIGN;
elem: (OP_MINUS)? (INT | HEX | ident | macro 
      | (LPAREN expression RPAREN));
expression_i: elem (op elem)*;
expression: expression_i (QMARK expression_i COLON expression_i)?;
assignment: ident assign_op expression SEMICOLON;
provide: (TOKEN_PROVIDE | TOKEN_PROVIDE_HIDDEN) LPAREN ident EQUALS expression RPAREN;
macro: (func LPAREN expression (COMMA expression)* RPAREN) | provide;


/* section blocks describe the linkers output */
actual_section: section:ident {
    if getattr(self, "section_names", None) == None: 
        self.section_names = []
    if #section.getText() not in self.section_names:
        self.section_names.append(#section.getText())
    };
input_section: ident LPAREN (assignment|(input_section|actual_section)) 
                       ((COMMA)? (input_section|actual_section))* RPAREN;
output_attributes: (OPERATOR ident)? (COLON ident)* (EQUALS expression)?;
output_section: (ident|DISCARD) (expression)? (TOKEN_BLOCK LPAREN expression RPAREN)? (NOLOAD)?  
                COLON (TOKEN_AT)? (expression)? 
                LCURLY (assignment | macro | input_section | SEMICOLON)* RCURLY 
                output_attributes;
overlay_section: ident LCURLY (assignment | macro | input_section | SEMICOLON)* RCURLY
                (COLON ident)* (EQUALS expression)?;
overlay_block: TOKEN_OVERLAY expression COLON (TOKEN_NOCROSSREFS)? (ident LPAREN expression RPAREN)?
                LCURLY overlay_section RCURLY output_attributes;
section_lines: macro | assignment | output_section | overlay_block| SEMICOLON;
section_block: TOKEN_SECTIONS LCURLY (section_lines)* RCURLY;

/* phdrs blocks describe the segments in the output file */
phdrs_lines: name:ident {
    if getattr(self, "segment_names", None) == None: 
        self.segment_names = []
    if #name.getText() not in self.segment_names:
        self.segment_names.append(#name.getText())
    } ident ((ident|TOKEN_PHDRS) (LPAREN expression RPAREN)?)* SEMICOLON;
phdrs_block: TOKEN_PHDRS LCURLY (phdrs_lines)* RCURLY;

/* mem blocks describe the memory layout of the target */
mem_lines: ident LPAREN (op|ident)* RPAREN COLON ident EQUALS expression COMMA ident EQUALS expression;
mem_block: TOKEN_MEMORY LCURLY (mem_lines)+ RCURLY;

/* the types of blocks in the file */
block: macro | assignment | section_block | phdrs_block | mem_block | SEMICOLON;

/* The entire script */
script: (block)* EOF;

