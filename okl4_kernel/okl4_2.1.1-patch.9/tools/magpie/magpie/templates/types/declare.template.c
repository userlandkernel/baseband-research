!!explicit_linebreaks isolated_scopes
/*-# We expect an "ast" parameter containing a type.
   # This is the top-level declaration, so just pass it down the tree.

handler = 'types_declare_%s' % (ast.get_attribute('meta_type')[0])
-*/
/*-run(templates.get(handler), ast = ast)-*/;\n
