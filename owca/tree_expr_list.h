#ifndef _RC_Y_TREE_EXPR_LIST_H
#define _RC_Y_TREE_EXPR_LIST_H


#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_expression_list;
	}
}

namespace owca { namespace __owca__ {
class tree_expression_list : public tree_expression {
	bool is_comprehension() const;
public:
	enum Opcode {
		OPCODE_CREATE_ARRAY,
		OPCODE_CREATE_TUPLE,
		OPCODE_CREATE_MAP,
		OPCODE_CREATE_SET,
		OPCODE_FUNCTION_CALL,
		OPCODE_FUNCTION_CALL_LM,
		OPCODE_CMP,
	};
	struct node {
		std::string ident;
		tree_expression *o;
		operatorcodes opc;
		node(tree_expression *o_, std::string ident_="") : o(o_),ident(ident_),opc((operatorcodes)0) { }
		node(tree_expression *o_, operatorcodes opc_) : o(o_),opc(opc_) { }
		~node() { }
	};
	Opcode opc;
	std::list<node> elems;
	tree_expression *mapvalue,*listvalue;
	tree_expression_list(compiler *cmp_, unsigned int l, Opcode opc_);
	~tree_expression_list() {
		while(!elems.empty()) {
			delete elems.front().o;
			elems.pop_front();
		}
		delete mapvalue;
		delete listvalue;
	}
	void compile_names(assignment_type assigned);
	bool compile_write(opcode_writer &dst, expression_type et);
	unsigned int get_first_location() const;
	unsigned int get_last_location() const;
	bool is_tuple() const { return opc==OPCODE_CREATE_TUPLE; }
};
} }
#endif
