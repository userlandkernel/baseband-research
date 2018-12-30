#include "ASTNode.hpp"
#include <assert.h>

ASTNode::ASTNode(const char *new_name)
{
	this->name = new_name;
}

ASTNode::~ASTNode()
{
}

void
ASTNode::forget_children()
{
	/* Forget about children without deleting them
	 * (IE, someone else is managing child memory
	*/
	this->children.clear();
}

void
ASTNode::delete_subtree()
{
	std::vector<ASTNode *>::iterator kiditer;
	for(kiditer = this->children.begin(); kiditer != this->children.end(); kiditer++) {
		(*kiditer)->delete_subtree();
		delete (*kiditer);
	}
	this->children.clear();
}

void
ASTNode::set_name(const char *new_name)
{
	this->name = new_name;
}

bool
ASTNode::has_name(const char *rhs_name)
{
	std::string rhs_string(rhs_name);

	return this->name == rhs_string;
}

void
ASTNode::set_value(const char *new_value)
{
	this->value = new_value;
}

void
ASTNode::extend_leaf(const char *extra_leaf)
{
	this->leaf += extra_leaf;
}

void
ASTNode::extend_value(const char *extra_value)
{
	this->value += extra_value;
}

void
ASTNode::set_leaf(const char *new_leaf)
{
	this->leaf = new_leaf;
}

bool
ASTNode::has_leaf(const char *rhs_leaf)
{
	std::string rhs_string(rhs_leaf);

	return this->leaf== rhs_string;
}

bool
ASTNode::has_any_leaf(void)
{
	return (this->leaf.size() != 0);
}

void
ASTNode::add_child(ASTNode *new_child)
{
	this->children.push_back(new_child);
}

bool
ASTNode::has_child(const char *child_name)
{
	std::vector<ASTNode *>::iterator kiditer;

	for(kiditer = this->children.begin(); kiditer != this->children.end(); kiditer++) {
		if((*kiditer)->has_name(child_name))
			return true;
	}
	return false;
}

ASTNode *
ASTNode::get_child(const char *child_name)
{
	std::vector<ASTNode *>::iterator kiditer;

	for(kiditer = this->children.begin(); kiditer != this->children.end(); kiditer++) {
		if((*kiditer)->has_name(child_name))
			return *kiditer;
	}
	return NULL;
}

ASTNode *
ASTNode::call_children(const char *child_name, void(*callback)(ASTNode *, void *), void *data)
{
	std::vector<ASTNode *>::iterator kiditer;

	for(kiditer = this->children.begin(); kiditer != this->children.end(); kiditer++) {
		if((*kiditer)->has_name(child_name))
			callback(*kiditer, data);
	}
	return NULL;
}

void
ASTNode::print_tree(int depth)
{
	int i;


	for(i = 0; i < depth; i++) {
		printf(" ");
	}

	printf("%s %s (%p)", this->name.c_str(), this->leaf.c_str(), this);
	if(!(this->value.empty()))
		printf(" = %s", this->value.c_str());
	
	printf("\n");

	std::vector<ASTNode *>::iterator kiditer;
	for(kiditer = this->children.begin(); kiditer != this->children.end(); kiditer++) {
		(*kiditer)->print_tree(depth + 1);
	}
}

void
ASTNode::print_tree(void)
{
	this->print_tree(0);
}

