!!explicit_linebreaks isolated_scopes
/*-command, arguments = command -*/

/*-if command == 'set_varptr'*/
mr_varptr = &mr[/*-?arguments[0]-*/];\n
/*fi-*/

Marshal this!\n
