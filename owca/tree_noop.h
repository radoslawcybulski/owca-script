#ifndef _RC_Y_TREE_NOOP_H
#define _RC_Y_TREE_NOOP_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_flow;
		class tree_noop;
	}
}

namespace owca { namespace __owca__ {
	class tree_noop : public tree_flow {
	public:
		tree_noop(compiler *cmp_, unsigned int l) : tree_flow(cmp_,l) { }
		bool compile_names();
		bool compile_write(opcode_writer &dst);
		unsigned int get_first_location() const { return location; }
		unsigned int get_last_location() const { return location; }
	};
} }
#endif
