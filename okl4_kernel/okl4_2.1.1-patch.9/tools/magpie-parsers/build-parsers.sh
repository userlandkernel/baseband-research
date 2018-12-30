#!/usr/bin/env bash

ANTLR=${ANTLR:-'runantlr'}

if [[ ! -x `which $ANTLR` ]]; then
echo \$ANTLR is set to \"$ANTLR\", which isn\'t executable. Exiting with bad grace.
exit;
fi

echo Compiling parsers
cd src/magpieparsers/cplusplus
# Fixme: We want to generate the Python parser from the C++ one eventually
#python cpp_to_python.py
#$ANTLR CPP_parser_py.g
# ... but for now use the separate Python parser
$ANTLR CPP_parser.g
$ANTLR CPP_parser_cpp.g
cd ../prolog
$ANTLR prolog.g
cd ../corba3
$ANTLR idl.g
cd ../../..

