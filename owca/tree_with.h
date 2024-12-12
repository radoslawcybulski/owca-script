#ifndef _RC_Y_TREE_WITH_H
#define _RC_Y_TREE_WITH_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_flow;
		class tree_with;
	}
}

namespace owca { namespace __owca__ {
class tree_with : public tree_flow {
public:
	struct tt {
		tree_expression *expr;
		std::string ident;
		tt() : expr(NULL) { }
	};
	std::list<tt> exprs;
	tree_flow *mainblock;
	tree_with(compiler *cmp_, unsigned int l) : tree_flow(cmp_,l),mainblock(NULL) { }
	~tree_with() {
		for(std::list<tt>::iterator it=exprs.begin();it!=exprs.end();++it) delete it->expr;
		delete mainblock;
	}
	bool compile_names();
	bool compile_write(opcode_writer &dst);
	unsigned int get_first_location() const;
	unsigned int get_last_location() const;
};
} }
#endif
