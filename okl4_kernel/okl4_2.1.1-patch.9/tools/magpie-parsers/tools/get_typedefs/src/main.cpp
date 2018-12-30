#include <istream>

#include "ASTNode.hpp"
#include "CPPLexer.hpp"
#include "CPPParser.hpp"

/* Find typdefs, which look like this:
 * declaration
 *  declaration_specifiers
 *   typedef
 *   [...] (other type specifiers)
 *  init_declarator_list
 *   init_declarator
 *    declarator
 *     direct_declarator <typedef name>
*/

void
handle_declaration_specifiers(ASTNode *node, void *data)
{
	bool *in_typedef = (bool *)data;

	if(node->has_child("typedef"))
		*in_typedef = true;
	else
		*in_typedef = false;
}

void
handle_direct_declarator(ASTNode *node, void *data)
{
	if(node->has_any_leaf())
		std::cout << node->get_leaf() << '\n';
}

void
handle_declarator(ASTNode *node, void *data)
{
	node->call_children("direct_declarator", handle_direct_declarator, NULL);
}

void
handle_init_declarator(ASTNode *node, void *data)
{
	node->call_children("declarator", handle_declarator, NULL);
}

void
handle_init_declarator_list(ASTNode *node, void *data)
{
	node->call_children("init_declarator", handle_init_declarator, NULL);
}

void
handle_declaration(ASTNode *node, void *data)
{
	bool in_typedef = false;

	node->call_children("declaration_specifiers", handle_declaration_specifiers, &in_typedef);
	if(in_typedef)
		node->call_children("init_declarator_list", handle_init_declarator_list, NULL);
}

void
print_typedefs(ASTNode *node)
{
	node->call_children("declaration", handle_declaration, NULL);
}

int
main(int argc,char* argv[])
{
	/* Create lexer and parser instances and get ready to parse */
	CPPLexer lexer(std::cin);
	lexer.setFilename("stdin");
	CPPParser parser(lexer);
	parser.setFilename("stdin");
	parser.init();

	/* Grab a parse tree and print what we want from it. */
	ASTNode *result = parser.translation_unit();
	print_typedefs(result);

	/* Done: clean up */
	result->delete_subtree();
	delete result;
	parser.uninit();

	return 0;
}

/* Dodgy global required by parser */
void
process_line_directive(const char *includedFile, const char *includedLineNo)
{
}

