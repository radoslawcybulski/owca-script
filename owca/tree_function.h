#ifndef _RC_Y_TREE_FUNCTION_H
#define _RC_Y_TREE_FUNCTION_H


#include "tree_base.h"

#define MAX_FUNCTION_PARAMS 64

namespace owca {
	class owca_location;
	namespace __owca__ {
		struct defval;
		class opcode_writer;
		class source;
		class compiler;
		class tree_executable;
		class tree_expression;
		class tree_function;
		class tree_varspace_location;
	}
}

namespace owca {
	namespace __owca__ {
		class tree_function : public tree_executable {
		public:
			struct paraminfo {
				std::string name;
				tree_expression *defval;
				tree_varspace_location *vi;
				unsigned int location;
				paraminfo() : defval(NULL),vi(NULL) { }
				~paraminfo() { }
			};
			std::list<paraminfo> params;
			paraminfo listparam,mapparam;
			bool generator;
			tree_varspace_location *selfvar;
			enum functiontype {
				FUNCTION,
				READ,
				WRITE
			} functype;
			enum functionmode {
				M_FUNCTION,
				M_SELF,
				M_CLASS,
				M_SCOPE_CREATOR,
				M_CLASS_CREATOR,
				M_COMPREHENSION,
			} funcmode;
			tree_flow *body;

			tree_function(compiler *cmp_, unsigned int l) : tree_executable(cmp_,l),generator(false),selfvar(NULL),functype(FUNCTION),body(NULL) { }
			~tree_function() {
				delete body;
				while(!params.empty()) {
					delete params.front().defval;
					params.pop_front();
				}
			}
			void tree_compile_params(source &s, const std::string &finaltoken);
			void compile_names(assignment_type assigned);
			bool compile_write(opcode_writer &dst, expression_type et);
			void compile_write_root(opcode_writer &dst);
		};
	}
}
#endif
