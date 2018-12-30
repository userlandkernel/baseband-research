!!explicit_linebreaks isolated_scopes
/*-if ast.has_attribute('c_name')*/
/*-?ast.get_attribute['c_name'][0]-*/
/*fi-*/
/*-if not ast.has_attribute('c_name')*/
/*-?ast.leaf-*/
/*fi-*/
