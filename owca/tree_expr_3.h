#ifndef _RC_Y_TREE_EXPR_3_H
#define _RC_Y_TREE_EXPR_3_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_expression_3;
	}
}

namespace owca { namespace __owca__ {
class tree_expression_3 : public tree_expression {
public:
	enum Opcode {
		OPCODE_QUE,
		OPCODE_ACCESS_2,
	};
	Opcode opc;
	tree_expression *o1,*o2,*o3;
	tree_expression_3(compiler *cmp_, unsigned int l, Opcode oopc, tree_expression *oo1, tree_expression *oo2, tree_expression *oo3) : tree_expression(cmp_,l),opc(oopc),o1(oo1),o2(oo2),o3(oo3) { }
	~tree_expression_3() { delete o1; delete o2; delete o3; }
	void compile_names(assignment_type assigned);
	bool compile_write(opcode_writer &dst, expression_type et);
	unsigned int get_first_location() const { return o1->get_first_location(); }
	unsigned int get_last_location() const { return o3->get_last_location(); }
};
} }
#endif
