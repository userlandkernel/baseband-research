!!explicit_linebreaks isolated_scopes
/* mr_varptr = magpie_unmarshal_smallstring(mr_varptr, {*-?args[0]-*}); */\n
/*-?args[0]-*/ = (char *)mr_varptr;\n
while(*mr_varptr) mr_varptr++;\n
mr_varptr++;
