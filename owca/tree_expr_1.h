#ifndef _RC_Y_TREE_EXPR_1_H
#define _RC_Y_TREE_EXPR_1_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_expression_1;
	}
}

namespace owca { namespace __owca__ {
class tree_expression_1 : public tree_expression {
public:
	enum Opcode {
		OPCODE_SIGN_CHANGE,
		OPCODE_BIN_NOT,
		OPCODE_LOG_NOT,
	};
	Opcode opc;
	tree_expression *o1;
	tree_expression_1(compiler *cmp_, unsigned int l, Opcode oopc, tree_expression *oo1) : tree_expression(cmp_,l),opc(oopc),o1(oo1) { }
	~tree_expression_1() { delete o1; }
	void compile_names(assignment_type assigned);
	bool compile_write(opcode_writer &dst, expression_type et);

	unsigned int get_first_location() const { return o1==NULL || location<o1->get_first_location() ? location : o1->get_first_location(); }
	unsigned int get_last_location() const { return o1==NULL || o1->get_last_location()<location ? location : o1->get_last_location(); }
};
} }
#endif
