#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_class_int.h"
#include "exec_coroutine.h"
#include "exec_object.h"

namespace owca { namespace __owca__ {

	//void exec_coroutine_object::_create(virtual_machine &, unsigned int oversize)
	//{
	//}

	void exec_coroutine_object::_marker(const gc_iteration &gc) const
	{
		if (_stack) {
			gc_iteration::debug_info _d("exec_coroutine_object %x: stack object",this);
			_stack->gc_mark(gc);
		}
	}

	void exec_coroutine_object::_destroy(virtual_machine &vm)
	{
		if (_stack) _stack->gc_release(vm);
	}

	D_SELF_(coroutine,init,exec_coroutine_object*)
		{
			if (cp.normal_params_count+cp.list_params_count==0) {
				vm->raise_not_enough_parameters(fnc);
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			const exec_variable *fnc;

			if (cp.normal_params_count>0) {
				fnc=cp.normal_params;
				cp.normal_params++;
				--cp.normal_params_count;
			}
			else {
				fnc=cp.list_params;
				cp.list_params++;
				--cp.list_params_count;
			}

			if (fnc->mode()!=VAR_FUNCTION && fnc->mode()!=VAR_FUNCTION_FAST) {
				vm->raise_invalid_param(*fnc,vm->class_function);
				return executionstackreturnvalue::FUNCTION_CALL;
			}

			{
				vmstack _vmstack(*vm);

				if (!vm->prepare_call_function(NULL,*fnc,cp,NULL)) {
					return executionstackreturnvalue::FUNCTION_CALL;
				}

				if (self->coroutine()) self->coroutine()->gc_release(*vm);
				self->set_coroutine(vm->execution_stack);
				vm->execution_stack->set_coroutine_object(v_self.get_object());
				vm->execution_stack->gc_acquire();
			}
			return_value->set_null(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(coroutine,str,exec_coroutine_object*)
		{
			return_value->set_string(vm->allocate_string("coroutine"));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(coroutine,bool,exec_coroutine_object*)
		{
			return_value->set_bool(self->coroutine()->count()!=0);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(coroutine,eq,exec_coroutine_object*,exec_coroutine_object*)
		{
			return_value->set_bool(self==p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(coroutine,noteq,exec_coroutine_object*,exec_coroutine_object*)
		{
			return_value->set_bool(self!=p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_coroutine(internal_class &c)
	{
		c._setinheritable(true);
		c._setstructure<exec_coroutine_object>();

		M_OPER_(c,coroutine,init,(*this,"$init"));
		M_OPER0(c,coroutine,str,(*this,"$str"));
		M_OPER0(c,coroutine,bool,(*this,"$bool"));
		M_OPER1(c,coroutine,eq,(*this,"$eq","other"));
		M_OPER1(c,coroutine,noteq,(*this,"$noteq","other"));

	}
} }












