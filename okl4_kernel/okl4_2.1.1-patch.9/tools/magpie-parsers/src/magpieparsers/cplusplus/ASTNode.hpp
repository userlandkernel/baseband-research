#ifndef ASTNode_hpp
#define ASTNode_hpp

#include <string>
#include <vector>

class ASTNode
{
private:
	std::string name, leaf, value;
	std::vector<ASTNode *>children;

public:
	ASTNode(const char *);
	~ASTNode();

private:
	ASTNode(const ASTNode &rhs) {}
	bool operator=(const ASTNode &rhs) {return false;}

public:
	void set_name(const char *);
	bool has_name(const char *);

	void set_value(const char *);
	void extend_value(const char *);

	void set_leaf(const char *);
	bool has_leaf(const char *);
	bool has_any_leaf(void);
	void extend_leaf(const char *);

	void add_child(ASTNode *);
	bool has_child(const char *);
	ASTNode *get_child(const char *);
	ASTNode *call_children(const char *, void(*)(ASTNode *, void *), void *data);

	const char *get_name(void) {return this->name.c_str();}
	const char *get_leaf(void) {return this->leaf.c_str();}
	const char *get_value(void) {return this->value.c_str();}
	int num_children(void) {return this->children.size();}
	ASTNode *get_child(int num) {return this->children[num];}

	void print_tree(void);

	void forget_children(void);
	void delete_subtree(void);
private:
	void print_tree(int);
};

#endif
