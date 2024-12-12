#ifndef _RC_Y_EXPR_LOOP_CONTROL_H
#define _RC_Y_EXPR_LOOP_CONTROL_H

#include "tree_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class opcode_writer;
		class compiler;
		class tree_flow;
		class tree_loop_control;
		class tree_varspace_location;
	}
}

namespace owca { namespace __owca__ {
	class tree_loop_control : public tree_flow {
	public:
		enum Type {
			BREAK,CONTINUE,RESTART,FINALLY
		};
		unsigned int location_end;
		std::string ident;
		Type type;
		tree_varspace_location *identloc;
		tree_loop_control(compiler *cmp_, Type type_, unsigned int begin, unsigned int end, const std::string &ident_) : tree_flow(cmp_,begin),type(type_),location_end(end),ident(ident_) { }
		bool compile_names();
		bool compile_write(opcode_writer &dst);
		unsigned int get_first_location() const { return location; }
		unsigned int get_last_location() const { return location_end; }
	};
} }
#endif
