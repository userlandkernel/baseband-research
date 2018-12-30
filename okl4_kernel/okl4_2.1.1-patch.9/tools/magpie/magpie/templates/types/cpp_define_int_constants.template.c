!!explicit_linebreaks
/*-
 # Define all integer constants as C preprocessor #defines.  
 # This can be used to get around the fact that you can't switch on const
 # ints in C, but only on int literals.
-*/
/*LOOP type_ast = generator.search( ['type_instance', {'meta_type': 'const'}] )*/
/*-basic_type = type_ast.the('target').the('type')-*/
/*-if basic_type.leaf in ('int', 'unsigned int', 'long', 'unsigned long', 'signed int', 'signed long')*/
#define _C_/*-?type_ast.leaf.upper()-*/ /*-?flatten(type_ast.the('declarator').the('expression'))-*/\n
#define /*-?type_ast.leaf.upper()-*/ /*-?flatten(type_ast.the('declarator').the('expression'))-*/\n
/*fi-*/
/*ENDLOOP*/
