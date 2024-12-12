#ifndef _RC_Y_OP_FLOW_DATA_OBJECT_H
#define _RC_Y_OP_FLOW_DATA_OBJECT_H

namespace owca {
	class gc_iteration;
	namespace __owca__ {
		struct vm_execution_stack_elem_internal;
		class virtual_machine;

		class op_flow_data_object {
			friend class exec_generator_function;
			friend class exec_function_stack_data;
			friend class virtual_machine;
		protected:
			op_flow_data_object() { }
		public:
			virtual ~op_flow_data_object() { }

			virtual unsigned int size() const=0;
			virtual bool resume_fin(vm_execution_stack_elem_internal &oe) { return false; }
			virtual bool resume_return(vm_execution_stack_elem_internal &oe) { return false; }
			virtual bool resume_loop_control(vm_execution_stack_elem_internal &oe) { return false; }
			virtual bool resume_exception(vm_execution_stack_elem_internal &oe) { return false; }

			virtual void _mark_gc(const gc_iteration &gc) const=0;
			virtual void _release_resources(virtual_machine &vm)=0;
		};
	}
}

#endif
