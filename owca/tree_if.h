#ifndef _RC_Y_TREE_IF_H
#define _RC_Y_TREE_IF_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_flow;
		class tree_if;
	}
}

namespace owca { namespace __owca__ {
class tree_if : public tree_flow {
public:
	struct pair {
		tree_expression *cond;
		tree_flow *block;
		pair(tree_expression *cond_, tree_flow *block_) : cond(cond_),block(block_) { }
		~pair() { }
	};
	tree_flow *elseblock,*finallyblock;
	std::list<pair> blocks;
	tree_if(compiler *cmp_, unsigned int l) : tree_flow(cmp_,l),elseblock(NULL),finallyblock(NULL) { }
	~tree_if() {
		while(!blocks.empty()) {
			delete blocks.front().cond;
			delete blocks.front().block;
			blocks.pop_front();
		}
		delete elseblock; delete finallyblock;
	}
	bool compile_names();
	bool compile_write(opcode_writer &dst);
	unsigned int get_first_location() const;
	unsigned int get_last_location() const;
};
} }
#endif
