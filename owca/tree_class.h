#ifndef _RC_Y_TREE_CLASS_H
#define _RC_Y_TREE_CLASS_H


#include "tree_base.h"
#include "tree_function.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_class;
		class tree_executable;
		class tree_expression;
		class tree_function;
	}
}

namespace owca { namespace __owca__ {
	class tree_class : public tree_executable {
	public:
		tree_function *body;

		std::list<tree_expression*> inherited;
		tree_class(compiler *cmp_, unsigned int l) : tree_executable(cmp_,l),body(NULL) { }
		~tree_class() {
			delete body;

			while(!inherited.empty()) {
				delete inherited.front();
				inherited.pop_front();
			}
		}
		void compile_names(assignment_type assigned);
		bool compile_write(opcode_writer &dst, expression_type et);
	};
} }
#endif
