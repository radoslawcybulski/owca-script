#ifndef _RC_Y_TREE_IDENT_H
#define _RC_Y_TREE_IDENT_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_expression_ident;
		class tree_varspace_location;
	}
}

namespace owca { namespace __owca__ {
	class tree_expression_ident : public tree_expression {
	public:
		std::string ident;
		bool function_class;
		assignment_type assigntype;
		tree_varspace_location *var;
		tree_expression_ident(compiler *cmp_, unsigned int l, const std::string &id, bool function_class_) : tree_expression(cmp_,l),ident(id),function_class(function_class_),var(NULL) { }
		~tree_expression_ident() { }
		void compile_names(assignment_type assigned);
		bool compile_write(opcode_writer &dst, expression_type et);
		unsigned int get_first_location() const { return location; }
		unsigned int get_last_location() const { return location; }
	};
} }
#endif
