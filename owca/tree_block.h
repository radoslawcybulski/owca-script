#ifndef _RC_Y_TREE_BLOCK_H
#define _RC_Y_TREE_BLOCK_H

#include "tree_base.h"
#include "op_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_block;
		class tree_flow;
	}
}

namespace owca { namespace __owca__ {
class tree_block : public tree_flow {
public:
	std::list<tree_flow*> elements;
	tree_block(compiler *cmp_, unsigned int l) : tree_flow(cmp_,l) { }
	~tree_block() {
		while(!elements.empty()) {
			delete elements.front();
			elements.pop_front();
		}
	}
	bool compile_names();
	bool compile_write(opcode_writer &dst);
	unsigned int get_first_location() const { return elements.size()>0 ? elements.front()->get_first_location() : location; }
	unsigned int get_last_location() const { return elements.size()>0 ? elements.back()->get_last_location() : location; }
};
} }
#endif
