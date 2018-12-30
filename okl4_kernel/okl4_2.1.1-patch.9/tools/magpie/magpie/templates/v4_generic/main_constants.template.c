!!explicit_linebreaks isolated_scopes
/*-run(templates.get('header_comments'))-*/
\n
/* IDL-defined constants */\n
\n
/*LOOP my_type_ast = generator.search( ['type_instance', {'meta_type': 'const'}] ) */
/*-run(templates.get('types_declare'), ast = my_type_ast)-*/
/*ENDLOOP*/
/*LOOP my_interface = generator.search( ['interface'] )*/
/*LOOP my_type_ast = my_interface.search( ['type_instance', {'meta_type': 'const'}] ) */
/*-run(templates.get('types_declare'), ast = my_type_ast)-*/
/*ENDLOOP*/
/*ENDLOOP*/
