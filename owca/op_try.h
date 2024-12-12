#ifndef _RC_Y_OP_TRY_H
#define _RC_Y_OP_TRY_H

#include "op_base.h"
#include "op_execute.h"
#include "exec_base.h"
#include "exec_function_stack_data.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca {
	namespace __owca__ {
		class opcode_executer_jump;
		class op_flow_data_object;
		class op_flow_try;
		class exec_object;
	}
}

namespace owca { namespace __owca__ {
	class op_flow_try : public op_flow_data_object {
	public:
		vm_execution_stack_elem_internal_jump blocks,elseblock,finallyblock,done;
		vm_execution_stack_elem_internal_jump jmp1,jmp2;
		exec_object *old_being_handled;
		unsigned int blockcount,blocksubcount;
		unsigned char mode;

		unsigned int size() const { return sizeof(*this); }
		bool set_dest_else_finally(vm_execution_stack_elem_internal &oe, const vm_execution_stack_elem_internal_jump &);
		bool resume_fin(vm_execution_stack_elem_internal &oe);
		bool resume_exception(vm_execution_stack_elem_internal &oe);

		void _mark_gc(const gc_iteration &gc) const;
		void _release_resources(virtual_machine &vm);
	};
} }
#endif
