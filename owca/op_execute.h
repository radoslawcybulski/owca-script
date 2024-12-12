#ifndef _RC_Y_OP_EXECUTE_H
#define _RC_Y_OP_EXECUTE_H

#include "operatorcodes.h"
#include "op_flow_data_object.h"
#include "op_base.h"

namespace owca {
	class owca_location;
	namespace __owca__ {
		//template <class A> class exec_array;
		enum execopcode;
		class opcode_executer;
		class opcode_executer_jump;
		class opcode_validator;
		class opcode_writer;
		class returnvalueflow;
		class exec_function_ptr;
		class exec_function_stack_data;
		class exec_variable;
		class exec_variable_location;
		class virtual_machine;
		class owca_internal_string;
		class op_flow_data_pointer_base;
	}
}

namespace owca {
	namespace __owca__ {

		class opcode_executer_jump {
			friend class opcode_executer;
			friend class opcode_validator;
			unsigned int offset;
			opcode_executer_jump(unsigned int off) : offset(off) { }
		public:
			opcode_executer_jump() : offset(0) { }

			bool operator == (const opcode_executer_jump &a) const { return offset == a.offset; }
			bool operator != (const opcode_executer_jump &a) const { return offset != a.offset; }
		};

		class op_flow_data_pointer_base {
		protected:
			virtual_machine *vm;
			exec_function_stack_data *fsd;
			op_flow_data_object *a;
		public:
			op_flow_data_pointer_base() : a(NULL),fsd(NULL) { }
			~op_flow_data_pointer_base();

			void set(opcode_executer &, unsigned int size);
			void set(opcode_executer &oe, exec_function_stack_data *);
			void cancel_pop();
		};
		template <class A> class op_flow_data_pointer : public op_flow_data_pointer_base {
			op_flow_data_pointer(const op_flow_data_pointer &);
		public:
			op_flow_data_pointer() { }

			void set(opcode_executer &oe, exec_function_stack_data *f) {
				op_flow_data_pointer_base::set(oe,f);
			}
			// void set(opcode_executer &oe, unsigned int added_size=0) {
			// 	op_flow_data_pointer_base::set(oe,sizeof(A)+added_size);
			// 	a->type=A::TYPE;
			// }

			A *operator -> (void) { return (A*)a; }
			const A *operator -> (void) const { return (A*)a; }
		};

	}
}

#endif
