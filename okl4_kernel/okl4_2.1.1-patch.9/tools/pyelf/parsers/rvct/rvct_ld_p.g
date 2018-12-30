/*
 * @LICENCE("Open Kernel Labs, Inc.", "2007")@
 */

header {
# @LICENCE("Open Kernel Labs, Inc.", "2007")@
}

header "rvct_ld_p.__main__" {
    import rvct_ld_l
    import rvct_ld_p


    L = rvct_ld_l.Lexer()
    P = rvct_ld_p.Parser(L)
    P.setFilename(L.getFilename())

    ### Parse the input expression
    try:
        P.scatter_description()
    except antlr.ANTLRException, ex:
        print "*** error(s) while parsing."
        print ">>> exit(1)"
        import sys
        sys.exit(1)

    ast = P.getAST()

    if not ast:
        print "stop - no AST generated."
        sys.exit(0)

    print "NB this script should only be run directly for debugging purposes\n"
    print "The segment to section names mapping as seen in the rvct linker or \"scatter gather\" script provided on stdin are:\n"
    print P.segments_to_sections
}


options {
    mangleLiteralPrefix = "TOKEN_";
    language=Python;
}



class rvct_ld_p extends Parser;
options {
    importVocab=RVCT_LD;
    buildAST =true;
}

scatter_description
    : ( load_region_description ) * EOF
    ;

load_region_description
    : load_region_name region_address attribute_list (max_size)? load_region_description_a
    ;

load_region_description_a
    : LCURLY execution_region_description_list RCURLY
    ;

load_region_name
    : a:IDENT {  
    /* print "load_region_name found ", a.getText() */
    if getattr(self, "segments_to_sections", None) == None:
        self.segments_to_sections = {}
    self.segments_to_sections[a.getText()] = []
    self.current_segment = a.getText()
    
    }
    ;

region_address
    : base_address
    | PLUS offset
    ;

base_address
    : expr
    ;
    
offset
    : number
    ;

expr
    : atom ((PLUS) atom)*
    ;

atom
    : number
    | LPAREN expr RPAREN
    ; 

attribute_list
    : (position_option)? (compression_option)?
    ;

compression_option
    : TOKEN_NOCOMPRESS
    ;

position_option
    : TOKEN_ABSOLUTE
    | TOKEN_PI
    | TOKEN_RELOC
    | TOKEN_OVERLAY
    ;

max_size 
    : number
    ;

execution_region_description_list
    : (execution_region_description)*
    ;

execution_region_description
    : execution_region_name region_address execution_attribute_list (size_length)? execution_region_description_a 
    ;

execution_region_description_a
    : LCURLY (input_section_description)* RCURLY
    ;

execution_region_name
    : a:IDENT { 
    self.segments_to_sections[self.current_segment].append(a.getText())
        }
    | b:ANY    { 
    self.segments_to_sections[self.current_segment].append(a.getText())
        } 
    ;

execution_attribute_list
    : (position_option)? (compression_option)? (initialisation_option)?
    ;

initialisation_option
    : TOKEN_EMPTY
    | TOKEN_PADVALUE number
    | TOKEN_ZEROPAD
    | TOKEN_UNINT
    ;

size_length
    : max_size
    | length    
    ;

length
    : MINUS number
    ;

input_section_description_list
    : (input_section_description)+
    ;

input_section_description
    : a:IDENT input_section_attribute_list { /* print "IDENT input_section_description found ", a.getText() */ }
    | b:ANY input_section_attribute_list { /* print "ANY input_section_description found ", b.getText() */ }
    | c:STAR input_section_attribute_list { /* print "STAR input_section_description found ", c.getText() */ }
    ;

input_section_attribute_list
    : LPAREN input_section_attribute_list_items RPAREN
    ;

input_section_attribute_list_items
    : input_section_attribute_list_item (COMMA input_section_attribute_list_item)*
    ;

input_section_attribute_list_item
    : PLUS input_section_attribute
    | input_symbol_pattern
    ;

input_section_attribute
    : TOKEN_RO_CODE
    | TOKEN_CODE
    | TOKEN_RO_DATA
    | TOKEN_RO
    | TOKEN_TEXT
    | TOKEN_RW_DATA
    | TOKEN_CONST
    | TOKEN_RW_CODE
    | TOKEN_RW
    | TOKEN_DATA
    | TOKEN_ZI
    | TOKEN_BSS
    | TOKEN_ENTRY
    | TOKEN_FIRST
    | TOKEN_LAST 
    ;


input_symbol_pattern
    : a:IDENT { 
     }
    | TOKEN_GDEF IDENT
    ;

number
    : INT
    | HEX
    ;
