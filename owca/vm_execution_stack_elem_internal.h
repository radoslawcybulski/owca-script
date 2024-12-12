#ifndef _RC_Y_VM_EXECUTION_STACK_ELEM_INTERNAL_H
#define _RC_Y_VM_EXECUTION_STACK_ELEM_INTERNAL_H

#include "debug_opcodes.h"
#include "vm_execution_stack_elem_base.h"
#include "exec_function_stack_data.h"
#include "op_base.h"
#include "operatorcodes.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		class virtual_machine;
		class exec_stack;
		class exec_function_ptr;
		class exec_function_stack_data;
		class op_flow_data_object;
		class exec_variable_location;
		class owca_internal_string;
		class opcode_data;
		class store_uint_or_ptr;
	}
}

namespace owca {
	namespace __owca__ {

		struct vm_execution_stack_elem_internal;

		class vm_execution_stack_elem_internal_jump {
			friend class virtual_machine;
			friend struct vm_execution_stack_elem_internal;
			unsigned int offset;
		public:
			vm_execution_stack_elem_internal_jump() : offset(0) { }
			//bool is_null() const { return offset==0; }
			unsigned int get_offset(void) const { return offset; }

			bool operator == (const vm_execution_stack_elem_internal_jump &j) const { return offset==j.offset; }
			bool operator != (const vm_execution_stack_elem_internal_jump &j) const { return offset!=j.offset; }
		};

		struct vm_execution_stack_elem_internal : public vm_execution_stack_elem_base {
			struct locationinfo {
				unsigned int line,offset_begin,offset_end;
			};

			virtual_machine *vm;
			exec_stack *stack;
			exec_function_stack_data *fncstackdata;
			opcode_data *opcodes;
			//const unsigned char *opcodes_data;
			unsigned int opcodes_offset;
			unsigned int tempstackmaxsize,tempstackactpos;
			unsigned int tempparamstrip;
			exec_object *exception_object_being_handled;

			vm_execution_stack_elem_internal() : tempparamstrip(0),exception_object_being_handled(NULL) { catch_exceptions(true); }
#ifdef RCDEBUG_OPCODES
			void ensure_type(opcodestreamtype type);
#else
			void ensure_type(opcodestreamtype type) { }
#endif

			vm_execution_stack_elem_internal &operator >> (operatorcodes &val);
			vm_execution_stack_elem_internal &operator >> (vm_execution_stack_elem_internal_jump &val);
			vm_execution_stack_elem_internal &operator >> (unsigned int &val);
			vm_execution_stack_elem_internal &operator >> (store_uint_or_ptr &val);
			vm_execution_stack_elem_internal &operator >> (owca_internal_string *&);
			vm_execution_stack_elem_internal &operator >> (owca_int &);
			vm_execution_stack_elem_internal &operator >> (owca_real &);
			vm_execution_stack_elem_internal &operator >> (exec_variable_location &);

			owca_location actual_location() const;
			vm_execution_stack_elem_internal_jump actual_code_position() const { vm_execution_stack_elem_internal_jump j; j.offset=opcodes_offset; return j; }
			void set_code_position(const vm_execution_stack_elem_internal_jump &j) { opcodes_offset=j.offset; }
			bool peek_exec();

			exec_variable &get_local(const exec_variable_location &loc);
			exec_variable &get0(unsigned short);
			exec_variable &get(const exec_variable_location &loc);

			template <class A> void push(A *&a, unsigned int oversize=0) { return fncstackdata->push(a,oversize); }
			void pop(op_flow_data_object *a);
			op_flow_data_object *peek();

			exec_variable &temp(unsigned int index);
			const exec_variable &temp(unsigned int index) const;

			executionstackreturnvalue first_time_execute(executionstackreturnvalue mode);
			executionstackreturnvalue execute(executionstackreturnvalue mode);

			void prepare_call_operator_stack(operatorcodes oper);
			void prepare_call_function_stack(unsigned int paramcnt);
			void prepare_call_function_stack(unsigned int cnt1, unsigned int cnt2, unsigned char mode);

			bool internal_frame() const { return true; }

		static unsigned int calculate_size(unsigned int tempvarcount);
		protected:
			void _release_resources(virtual_machine &vm);
			void _mark_gc(const gc_iteration &gc) const;
		};

	}
}

#endif
