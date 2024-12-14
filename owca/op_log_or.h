#ifndef _RC_Y_OP_LOG_OR_H
#define _RC_Y_OP_LOG_OR_H

#include "op_base.h"
#include "op_execute.h"
#include "exec_function_stack_data.h"
#include "returnvalueflow.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca {
	namespace __owca__ {
		class opcode_executer_jump;
		class exec_variable;
		class op_flow_data_object;
		class virtual_machine;
		struct vm_execution_stack_elem_internal;
		class vm_execution_stack_elem_internal_jump;
	}
}

namespace owca { namespace __owca__ {
	class op_flow_log_or : public op_flow_data_object {
	public:
		vm_execution_stack_elem_internal_jump jmp;
		unsigned char mode;
		exec_variable tmp;

		unsigned int size() const { return sizeof(*this); }
		bool update_bool_value(vm_execution_stack_elem_internal &oe);
		bool resume_fin(vm_execution_stack_elem_internal &oe);

		void _mark_gc(const gc_iteration &gc) const { }
		void _release_resources(virtual_machine &vm) { }
	};

} }

#endif