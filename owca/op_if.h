#ifndef _RC_Y_OP_IF_H
#define _RC_Y_OP_IF_H

#include "op_base.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca {
	namespace __owca__ {
		class opcode_executer_jump;
		class op_flow_data_object;
		class op_flow_if;
	}
}

namespace owca { namespace __owca__ {
	class op_flow_if : public op_flow_data_object {
	public:
		vm_execution_stack_elem_internal_jump next,elseblock,finallyblock,done;
		unsigned int count;
		unsigned char mode;

		bool set_dest_else_finally(vm_execution_stack_elem_internal &oe, vm_execution_stack_elem_internal_jump j);
		unsigned int size() const { return sizeof(*this); }
		bool resume_fin(vm_execution_stack_elem_internal &oe);

		void _mark_gc(const gc_iteration &gc) const { }
		void _release_resources(virtual_machine &vm) { }
	};
} }
#endif
