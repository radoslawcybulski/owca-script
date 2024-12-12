#ifndef _RC_Y_OP_WITH_H
#define _RC_Y_OP_WITH_H

#include "op_base.h"
#include "op_execute.h"
#include "exec_function_stack_data.h"
#include "returnvalueflow.h"
#include "exec_variable.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca {
	namespace __owca__ {
		class opcode_executer_jump;
		class exec_variable;
		class op_flow_data_object;
		class virtual_machine;
		struct vm_execution_stack_elem_internal;
	}
}

namespace owca { namespace __owca__ {
	class op_flow_with : public op_flow_data_object {
	public:
		op_flow_with() : back(returnvalueflow::FIN) { }

		exec_object *excobj;
		exec_variable tmp;
		returnvalueflow back;
		unsigned int cnt,act,mode;
		vm_execution_stack_elem_internal_jump start,resume;

		exec_variable *variables() { return (exec_variable*)(((char*)this)+sizeof(*this)); }
		const exec_variable *variables() const { return (const exec_variable*)(((const char*)this)+sizeof(*this)); }
		bool callvars_prepare(virtual_machine &vm);

		unsigned int size() const;
		bool resume_fin(vm_execution_stack_elem_internal &oe);
		bool resume_loop_control(vm_execution_stack_elem_internal &oe);
		bool resume_exception(vm_execution_stack_elem_internal &oe);
		bool resume_return(vm_execution_stack_elem_internal &oe);

		void _mark_gc(const gc_iteration &gc) const;
		void _release_resources(virtual_machine &vm);
	};
} }
#endif
