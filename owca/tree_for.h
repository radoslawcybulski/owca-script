#ifndef _RC_Y_TREE_FOR_H
#define _RC_Y_TREE_FOR_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_flow;
		class tree_for;
	}
}

namespace owca { namespace __owca__ {
class tree_for : public tree_flow {
public:
	std::string name;
	tree_expression *vars,*loop;
	tree_flow *mainblock,*elseblock,*finallyblock;
	tree_for(compiler *cmp_, unsigned int l) : tree_flow(cmp_,l),mainblock(NULL),elseblock(NULL),finallyblock(NULL),vars(NULL),loop(NULL) { }
	~tree_for() { delete vars; delete loop; delete mainblock; delete elseblock; delete finallyblock; }
	bool compile_names();
	bool compile_write(opcode_writer &dst);
	unsigned int get_first_location() const;
	unsigned int get_last_location() const;
};
} }
#endif
