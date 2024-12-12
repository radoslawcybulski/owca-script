#ifndef _RC_Y_TREE_EQ_H
#define _RC_Y_TREE_EQ_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_eq;
		class tree_expression;
		class tree_flow;
	}
}

namespace owca { namespace __owca__ {
	class tree_eq : public tree_flow {
	public:
		unsigned int location_end;
		tree_expression *root;
		tree_eq(compiler *cmp_, unsigned int beg, unsigned int end, tree_expression *expr) : tree_flow(cmp_,beg),location_end(end),root(expr) { }
		~tree_eq() {
			delete root;
		}
		bool compile_names();
		bool compile_write(opcode_writer &dst);
		unsigned int get_first_location() const { return location; }
		unsigned int get_last_location() const { return location_end; }
	};
} }
#endif
