"""
Want to generate CPP_parser.g from CPP_parser_cpp.g automatically (ie translate C++ 
actions to Python actions).
"""
import re

DATA_MATCHER = re.compile('(translation_unit returns.*)class CPPLexer', re.MULTILINE | re.DOTALL)

# A bunch of evil hacks because there are a few instances where the parser inspects state
# directly, and other stuff I can't be bothered translating cleverly
EVIL = [
	(r'{( !(LA(1)==SCOPE||LA(1)==ID) || qualifiedItemIsOneOf(qiType|qiCtor) )}',
			r'{( not(self.LA(1)==SCOPE or self.LA(1)==ID) or qualifiedItemIsOneOf(qiType|qiCtor) )}'),
	(r'{(!((LA(1)==LPAREN)&&(LA(2)==ID))||(qualifiedItemIsOneOf(qiType|qiCtor,1)))}',
			r'{(not((self.LA(1)==LPAREN) and (LA(2)==ID))or(qualifiedItemIsOneOf(qiType|qiCtor,1)))}'),
	(r'{!((LA(1)==SCOPE) && (LA(2)==STAR||LA(2)==OPERATOR))&&( !(LA(1)==SCOPE||LA(1)==ID) || qualifiedItemIsOneOf(qiType|qiCtor) )}',
			r'{not((self.LA(1)==SCOPE) and (self.LA(2)==STAR or self.LA(2)==OPERATOR)) and ( not(self.LA(1)==SCOPE or self.LA(1)==ID) or qualifiedItemIsOneOf(qiType|qiCtor) )}'),
	(r'{(!(((LA(1)==LPAREN&&(LA(2)==ID))))||(isTypeName((LT(2)->getText()).data())))}',
			r'{(not(((self.LA(1)==LPAREN and (self.LA(2)==ID))))or(isTypeName((self.LT(2).getText()))))}'),
	(r'{!(LA(1)==LPAREN)}', '{not (self.LA(1) == LPAREN)}'),
	(r'int taken_x4 = 0, taken_x5 = 0', r'taken_x4 = 0; taken_x5 = 0'),
	(r'(LT(1)->getText()).data()', 'self.LT(1).getText()'),
]

# Ordering is somewhat important in these replacements... in particular
# the replacement of ASTNode pointers in "returns" statements must go
# before we obliterate ASTNode pointers as declarations, and the "cleanup"
# regexes that eliminate whitespace and dead blocks must go at the end.
REPLACEMENTS = [(re.compile(source, re.MULTILINE | re.DOTALL), dest)
	for source, dest in [
	(r'_td', 'self._td'), # Class-wide locals
	(r'_fd', 'self._fd'), # Class-wide locals
	(r'_sc', 'self._sc'), # Class-wide locals
	(r'_tq', 'self._tq'), # Class-wide locals
	(r'_ts', 'self._ts'), # Class-wide locals
	(r'_ds', 'self._ds'), # Class-wide locals
	(r'functionDefinition', 'self.functionDefinition'), # Class-wide locals
	(r'assign_stmt_RHS_found', 'self.assign_stmt_RHS_found'), # Class-wide locals
	(r'in_parameter_list', 'self.in_parameter_list'), # Class-wide locals
	(r'K_and_R', 'self.K_and_R'), # Class-wide locals
	(r'in_return', 'self.in_return'), # Class-wide locals
	(r'is_address', 'self.is_address'), # Class-wide locals
	(r'is_pointer', 'self.is_pointer'), # Class-wide locals
	(r'exprnode_acc', 'self.exprnode_acc'), # Class-wide locals
	(r'qualifiedItemIsOneOf', 'self.qualifiedItemIsOneOf'), # Class-wide locals
	(r'enterExternalScope', 'self.enterExternalScope'), # Class-wide locals
	(r'exitExternalScope', 'self.exitExternalScope'), # Class-wide locals
	(r'beginEnumDefinition', 'self.beginEnumDefinition'), # Class-wide locals
	(r'beginFunctionDefinition', 'self.beginFunctionDefinition'), # Class-wide locals
	(r'endFunctionDefinition', 'self.endFunctionDefinition'), # Class-wide locals
	(r'beginParameterDeclaration', 'self.beginParameterDeclaration'), # Class-wide locals
	(r'endParameterDeclaration', 'self.endParameterDeclaration'), # Class-wide locals
	(r'functionParameterList', 'self.functionParameterList'), # Class-wide locals
	(r'functionEndParameterList', 'self.functionEndParameterList'), # Class-wide locals
	(r'enterNewLocalScope', 'self.enterNewLocalScope'), # Class-wide locals
	(r'exitLocalScope', 'self.exitLocalScope'), # Class-wide locals
	(r'declaratorParameterList', 'self.declaratorParameterList'), # Class-wide locals
	(r'declaratorEndParameterList', 'self.declaratorEndParameterList'), # Class-wide locals
	(r'declaratorArray', 'self.declaratorArray'), # Class-wide locals
	(r'scopedItem', 'self.scopedItem'), # Class-wide locals
	(r'enumElement', 'self.enumElement'), # Class-wide locals
	(r'endEnumDefinition', 'self.endEnumDefinition'), # Class-wide locals
	(r'end_of_stmt', 'self.end_of_stmt'), # Class-wide locals
	(r'beginDeclaration', 'self.beginDeclaration'), # Class-wide locals
	(r'endDeclaration', 'self.endDeclaration'), # Class-wide locals
	(r'beginClassDefinition', 'self.beginClassDefinition'), # Class-wide locals
	(r'endClassDefinition', 'self.endClassDefinition'), # Class-wide locals
	(r'beginFieldDeclaration', 'self.beginFieldDeclaration'), # Class-wide locals
	(r'declarationSpecifier', 'self.declarationSpecifier'), # Class-wide locals
	(r'enclosingClass', 'self.enclosingClass'), # Class-wide locals
	(r'declaratorID', 'self.declaratorID'), # Class-wide locals
	(r'classForwardDeclaration', 'self.classForwardDeclaration'), # Class-wide locals
	(r'isTypeName', 'self.isTypeName'), # Class-wide locals
	(r'\[[a-z]+ ([a-z_]+)\] returns', r'[\1] returns'), # No types on inherited attributes
	(r'returns ?\[ASTNode \*(\w+?)\]', r'returns [\1]'), # Fix all return values
	(r'returns ?\[char \*(\w+?)\]', r'returns [\1]'), # More: fix all return values
	(r'returns ?\[const char \*(\w+?)\]', r'returns [\1]'), # More: fix all return values
	(r'^(\w+?)\[DeclSpecifier', r'\1['), # Computed attribute types
	(r'mk_node', r'self.node'), # Different way to create nodes
	(r'(\w+?)->add_child\((.+?)\);', r'\1.add_child(\2);'), # No pointers in Python
	(r'(\w+?)->set_leaf', r'\1.set_leaf'), # No pointers in Python #2
	(r'(\w+?)->set_name', r'\1.set_name'), # No pointers in Python #3
	(r'(\w+?)->set_value', r'\1.set_value'), # No pointers in Python #4
	(r'(\w+?)->extend_value', r'\1.extend_value'), # No pointers in Python #5
	(r'\((\w+?)->getText\(\)\).data\(\)', r'\1.getText()'), # No pointers in Python #6
	(r'->getText\(\).data\(\)', r'.getText()'), # No pointers in Python #7
	(r'->getLine', r'.getLine'), # No pointers in Python #8
	(r'->extend_leaf', r'.extend_leaf'), # No pointers in Python #9
	(r'symbols->strdup\((\w+?)\)', r'\1'), # Python strings are immutable
	(r'ASTNode \*\w+?(, ?\*\w+?)*;', ''), # ASTNode declarations -> nothing
	(r'static char .*?;', ''), # Static C strings - nothing
	(r'char \*(\w+?) ?= ?NULL', r'\1 = ""'), # Declaration of char *
	(r'(const )?char .+?;', ''), # C strings -> nothing
	(r'bool (\w+?) ?=', r'\1 ='), # Declaration of Boolean
	(r'ASTNode \*(\w+?) ?=(.*?);', r'\1 = \2;'), # Declaration of ASTNode *
	(r'DeclSpecifier (\w+?) ?= ?(\w+?);', r'\1 = \2'), # Declaration of DeclSpecifier
	(r'TypeSpecifier (\w+?);', r''), # Declaration of TypeSpecifier
	(r'(\w)\[0\] ?=.*?;', r'\1 = ""'), # Initialisation of C string
	(r'strcat\((\w+?),(.*?)\);', r'\1 += \2;'), # C string concatenation
	(r'strcpy\((\w+?), ?(\w+?)\)', r'\1 = \2'), # C string copying
	(r'(?<!self.)LA\(1\)', 'self.LA(1)'), # Direct manipulation of parser
	(r'(?<!self.)LA\(2\)', 'self.LA(2)'), # Direct manipulation of parser
	(r'\{if ?\((.*?)\)', r'{if (\1):'), # Very basic if-conversion
	(r'(tqInvalid|tsInvalid|dsInvalid)', 'None'), # Declaration specifiers
	(r'tsSTRUCT', '"struct"'), # type specifier 
	(r'tsCLASS', '"class"'), # type specifier 
	(r'tsUNION', '"union"'), # type specifier 
	# FIXME: This is broken until we can distinguish between ANTLR "options"
	# slots and real code productions
	#(r';\}', '}'), # Python doesn't need semicolons to terminate instructions
	(r';;*?\}', ';}'), # At most one semi
	(r'\{[\r\n\t ]+', '{'), # Eliminate white space in actions
	(r'[\r\n\t ]+\}', '}'), # More whitespace
	(r'\{\}', ''), # Eliminate any empty code blocks we created
	]]

def go(filename):
	data = _get_data(filename)
	for source, dest in EVIL:
		data = data.replace(source, dest)
	for source, dest in REPLACEMENTS:
		data = re.sub(source, dest, data)
	
	template = file('CPP_parser_py.template').read()
	output = template.replace('PARSER_GOES_HERE', data)
	file('CPP_parser_py.g', 'wb').write(output)
	
def _get_data(filename):
	data = file(filename, 'rb').read()
	data = DATA_MATCHER.search(data)
	if data is None:
		raise Exception("Couldn't find parser in this file")
	return data.group(1)

def main():
	go('CPP_parser_cpp.g')

if __name__ == '__main__':
	main()
