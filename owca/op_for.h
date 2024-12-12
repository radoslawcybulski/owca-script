#ifndef _RC_Y_OP_FOR_H
#define _RC_Y_OP_FOR_H

#include "op_base.h"
#include "op_execute.h"
#include "exec_base.h"
#include "exec_function_stack_data.h"
#include "exec_variable.h"
#include "exec_variablelocation.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca {
	namespace __owca__ {
		class exec_variable;
		class exec_variable_location;
		class op_flow_data_object;
		class op_flow_for;
	}
}

namespace owca { namespace __owca__ {
	class op_flow_for : public op_flow_data_object {
	public:
		exec_variable_location counter;
		vm_execution_stack_elem_internal_jump mainblock,elseblock,finallyblock,begin,done;
		exec_variable *countervar;
		exec_variable generator,temp;
		owca_int countervalue;
		char firsttime;
		unsigned char mode;

		bool set_dest_else_finally(vm_execution_stack_elem_internal &oe);
		unsigned int size() const { return sizeof(*this); }
		bool resume_fin(vm_execution_stack_elem_internal &oe);
		bool resume_loop_control(vm_execution_stack_elem_internal &oe);

		void _mark_gc(const gc_iteration &gc) const;
		void _release_resources(virtual_machine &vm);
	};
} }
#endif
