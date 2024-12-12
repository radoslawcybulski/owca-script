#ifndef _RC_Y_TREE_TRY_H
#define _RC_Y_TREE_TRY_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_expression;
		class tree_flow;
		class tree_try;
		class tree_varspace_location;
	}
}

namespace owca { namespace __owca__ {
	class tree_try : public tree_flow {
	public:
		struct pair {
			tree_flow *block;
			std::vector<tree_expression*> types;
			std::string asvar;
			unsigned int location;
			tree_varspace_location *vi;
			pair() : block(NULL),vi(NULL) { }
		};
		tree_flow *mainblock,*elseblock,*finallyblock;
		std::list<pair> blocks;
		tree_try(compiler *cmp_, unsigned int l) : tree_flow(cmp_,l),mainblock(NULL),elseblock(NULL),finallyblock(NULL) { }
		~tree_try() {
			while(!blocks.empty()) {
				delete blocks.front().block;
				for(unsigned int i=0;i<blocks.front().types.size();++i) delete blocks.front().types[i];
				blocks.pop_front();
			}
			delete elseblock; delete finallyblock; delete mainblock;
		}
		bool compile_names();
		bool compile_write(opcode_writer &dst);
		unsigned int get_first_location() const;
		unsigned int get_last_location() const;
	};
} }
#endif
