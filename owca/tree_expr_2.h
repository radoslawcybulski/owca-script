#ifndef _RC_Y_TREE_EXPR_2_H
#define _RC_Y_TREE_EXPR_2_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_expression_2;
	}
}

namespace owca { namespace __owca__ {
class tree_expression_2 : public tree_expression {
public:
	enum Opcode {
		OPCODE_LOOKUP,
		OPCODE_ACCESS_1,
		OPCODE_ASSIGN,
		OPCODE_ASSIGN_PROPERTY,
		OPCODE_ASSIGN_NON_LOCAL,
		OPCODE_ASSIGN_ADD,
		OPCODE_ASSIGN_SUB,
		OPCODE_ASSIGN_MUL,
		OPCODE_ASSIGN_DIV,
		OPCODE_ASSIGN_MOD,
		OPCODE_ASSIGN_LSHIFT,
		OPCODE_ASSIGN_RSHIFT,
		OPCODE_ASSIGN_BIN_AND,
		OPCODE_ASSIGN_BIN_OR,
		OPCODE_ASSIGN_BIN_XOR,
		OPCODE_MUL,
		OPCODE_DIV,
		OPCODE_MOD,
		OPCODE_ADD,
		OPCODE_SUB,
		OPCODE_LSHIFT,
		OPCODE_RSHIFT,
		OPCODE_BIN_AND,
		OPCODE_BIN_OR,
		OPCODE_BIN_XOR,
		OPCODE_IS,
		OPCODE_IN,
		OPCODE_LOG_AND,
		OPCODE_LOG_OR,
		};
	Opcode opc;
	tree_expression *o1,*o2;
	tree_expression_2(compiler *cmp_, unsigned int l, Opcode oopc, tree_expression *oo1, tree_expression *oo2) : tree_expression(cmp_,l),opc(oopc),o1(oo1),o2(oo2) { }
	~tree_expression_2() { delete o1; delete o2; }

	void compile_names(assignment_type assigned);
	bool compile_write(opcode_writer &dst, expression_type et);
	unsigned int get_first_location() const { return o1->get_first_location(); }
	unsigned int get_last_location() const { return o2->get_last_location(); }
};
} }
#endif
