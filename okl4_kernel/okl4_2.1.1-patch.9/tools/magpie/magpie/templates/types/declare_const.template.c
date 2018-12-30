!!explicit_linebreaks isolated_scopes
/*-
const_value = flatten(ast.the('expression'))
const_value_type = "expression" # FIXME: This is a hack
const_type_ast = ast.the('target').the('type')

if ast.has_attribute('c_name'):
	const_name = ast.get_attribute('c_name')[0]
else:
	const_name = ast.leaf
-*/
const /*-run(templates.get('types_name'), ast = const_type_ast)-*/ /*-?const_name-*/ = /*-run(templates.get('types_constant'), value = const_value, value_type = const_value_type)-*/
