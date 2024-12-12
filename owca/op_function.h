#ifndef _RC_Y_OP_FUNCTION_H
#define _RC_Y_OP_FUNCTION_H

#include "op_base.h"
#include "exec_function_ptr.h"
#include "exec_function_stack_data.h"

namespace owca {
	namespace __owca__ {
		class vm_execution_stack_elem_internal_jump;
		struct vm_execution_stack_elem_internal;
	}
}

namespace owca { namespace __owca__ {

	//class op_flow_function : public op_flow_data_object {
	//public:
	//	exec_function_ptr *fnc;
	//	int index,count;
	//	vm_execution_stack_elem_internal_jump *code;
	//	exec_function_ptr::internal_param_info *params;

	//	unsigned int size() const;
	//	bool resume_fin(vm_execution_stack_elem_internal &oe);
	//	bool prepare(vm_execution_stack_elem_internal &oe);

	//	void _mark_gc(const gc_iteration &gc) const;
	//	void _release_resources(virtual_machine &vm);
	//};

} }

#endif
