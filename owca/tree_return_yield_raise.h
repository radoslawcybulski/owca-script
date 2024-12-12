#ifndef _RC_Y_TREE_RETURN_H
#define _RC_Y_TREE_RETURN_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_flow;
		class tree_return_yield_raise;
	}
}

namespace owca { namespace __owca__ {
	class tree_return_yield_raise : public tree_flow {
	public:
		enum Type {
			RETURN,YIELD,RAISE
		} type;
		tree_expression *value;
		tree_return_yield_raise(compiler *cmp_, unsigned int l, Type type_, tree_expression *expr) : tree_flow(cmp_,l),value(expr),type(type_) { }
		~tree_return_yield_raise() {
			delete value;
		}
		bool compile_names();
		bool compile_write(opcode_writer &dst);
		unsigned int get_first_location() const { return location; }
		unsigned int get_last_location() const { return value->get_last_location(); }
	};
} }
#endif
