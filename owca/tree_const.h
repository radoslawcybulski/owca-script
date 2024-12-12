#ifndef _RC_Y_TREE_CONST_H
#define _RC_Y_TREE_CONST_H

#include "tree_base.h"
#include "op_base.h"
#include "cmp_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class token;
		class compiler;
		class tree_expression;
		class tree_expression_const;
	}
}

namespace owca { namespace __owca__ {
	class tree_expression_const : public tree_expression {
	public:
		std::string value;
		enum type_ { T_INT,T_REAL,T_STRING,T_NULL,T_TRUE,T_FALSE,T_SELF,T_CLASS,T_SPEC } type;
		tree_expression_const(compiler *cmp_, token *t, type_ tt) : tree_expression(cmp_,t->line()),value(t->text()),type(tt) { }
		~tree_expression_const() { }
		void compile_names(assignment_type assigned);
		bool compile_write(opcode_writer &dst, expression_type et);
		unsigned int get_first_location() const { return location; }
		unsigned int get_last_location() const { return location; }
	};
} }
#endif
