#include "stdafx.h"
#include "base.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "exec_class_int.h"
#include "exec_function_ptr.h"
#include "op_base.h"
#include "exec_stack.h"
#include "returnvalue.h"
#include "exec_function_stack_data.h"
#include "op_execute.h"
#include "vm_execution_stack_elem_external.h"
#include "operatorcodes.h"

namespace owca { namespace __owca__ {

	D_SELF0(generator,call,vm_execution_stack_elem_base*)
		{
			RCASSERT(vm->execution_stack->peek_frame()==this);
			RCASSERT(self->return_handling_mode==vm_execution_stack_elem_base::RETURN_HANDLING_NONE);

			RCASSERT(vm->push_execution_stack_frame(self));
			self->gc_acquire();

			show_in_exception_stack(false);
			finalized(true);

			return executionstackreturnvalue::FUNCTION_CALL;
		}
	D_END

	D_SELF0(generator,gen,vm_execution_stack_elem_base*)
		{
			return_value->set_generator(self);
			self->gc_acquire();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_FUNC1(generator,new,const exec_variable *)
		{
			switch(mode) {
			CASE(0)
				show_in_exception_stack(false);
				CALC(1,vm->calculate_generator(return_value,*p1));
			CASE(1)
				break;
			default:
				RCASSERT(0);
			}
			return executionstackreturnvalue::RETURN;
		}
		unsigned char mode;
		void create_self(void) { mode=0; }
	D_END

	D_SELF0(generator,bool,vm_execution_stack_elem_base*)
		{
			return_value->set_bool(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(generator,str,vm_execution_stack_elem_base*)
		{
			return_value->set_string(vm->allocate_string("generator object"));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(generator,eq,vm_execution_stack_elem_base*,vm_execution_stack_elem_base*)
		{
		return_value->set_bool(self==p1);
		return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(generator,noteq,vm_execution_stack_elem_base*,vm_execution_stack_elem_base*)
		{
		return_value->set_bool(self!=p1);
		return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_generator(internal_class &c)
	{
		c._setinheritable(false);

		M_OPER1(c,generator,new,(*this,"$new","value"));
		M_OPER0(c,generator,call,(*this,"$call"));
		M_OPER0(c,generator,str,(*this,"$str"));
		M_OPER0(c,generator,bool,(*this,"$bool"));
		M_OPER0(c,generator,gen,(*this,"$gen"));

		M_OPER1(c,generator,eq,(*this,"$eq","other"));
		M_OPER1(c,generator,noteq,(*this,"$noteq","other"));
	}
} }












