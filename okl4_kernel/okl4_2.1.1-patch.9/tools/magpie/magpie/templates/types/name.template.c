!!explicit_linebreaks isolated_scopes
/*-if ast.has_attribute('c_name')*/
/*-?ast.get_attribute('c_name')[0]-*/
/*fi-*/

/*-if not ast.has_attribute('c_name')*/
/*-meta_type = ast.get_attribute('meta_type')[0]-*/
/*-run(templates.get('types_name_%s' % (meta_type)), ast = ast)-*/
/*fi-*/
