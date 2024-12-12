#ifndef _RC_Y_TREE_WHILE_H
#define _RC_Y_TREE_WHILE_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_flow;
		class tree_while;
	}
}

namespace owca { namespace __owca__ {
class tree_while : public tree_flow {
public:
	std::string name;
	tree_expression *condition;
	tree_flow *mainblock,*elseblock,*finallyblock;
	tree_while(compiler *cmp_, unsigned int l) : tree_flow(cmp_,l),mainblock(NULL),elseblock(NULL),finallyblock(NULL),condition(NULL) { }
	~tree_while() { delete condition; delete mainblock; delete elseblock; delete finallyblock; }
	bool compile_names();
	bool compile_write(opcode_writer &dst);
	unsigned int get_first_location() const;
	unsigned int get_last_location() const;
};
} }
#endif
