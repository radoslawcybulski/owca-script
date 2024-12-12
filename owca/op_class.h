#ifndef _RC_Y_OP_CLASS_H
#define _RC_Y_OP_CLASS_H

#include "exec_function_stack_data.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca {
	namespace __owca__ {
		class vm_execution_stack_elem_internal_jump;
		struct vm_execution_stack_elem_internal;
		class exec_stack;
		class owca_internal_string;
		class exec_object;
	}
}

namespace owca { namespace __owca__ {

	class op_flow_class : public op_flow_data_object {
	public:
		unsigned int inheritedcount;
		exec_stack *stack;
		exec_function_ptr *fnc;
		owca_internal_string *name;

		unsigned int size() const { return sizeof(*this); }
		bool resume_fin(vm_execution_stack_elem_internal &oe);

		void _mark_gc(const gc_iteration &gc) const;
		void _release_resources(virtual_machine &vm);
	};

} }

#endif
