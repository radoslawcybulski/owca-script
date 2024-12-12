#include "stdafx.h"
#include "base.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "exec_class_object.h"
#include "exec_map_object.h"
#include "exec_object.h"
#include "exec_class_int.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_external.h"
#include "exec_callparams.h"

namespace owca { namespace __owca__ {

	D_SELF_(class,call,exec_class_object*)
		{
			const exec_variable &slf=v_self;

			if (slf.get_object()==vm->class_class) {
				if (!vm->ensure_no_map_params(cp.map)) return executionstackreturnvalue::FUNCTION_CALL;
				if (cp.normal_params_count+cp.list_params_count==0) {
					return_value->set_object(vm->class_class);
					return_value->gc_acquire();
					return executionstackreturnvalue::RETURN;
				}
				else if (cp.normal_params_count+cp.list_params_count==1) {
					const exec_variable *p=cp.normal_params_count!=0 ? &cp.normal_params[0] : &cp.list_params[0];

					return_value->set_object(p->mode()==VAR_OBJECT ? p->get_object()->type() : vm->basicmap[p->mode()]);
					return_value->gc_acquire();
					return executionstackreturnvalue::RETURN;
				}
				else {
					RCASSERT(vm->execution_stack->peek_frame()==this);

					exec_function_ptr *f=fnc;
					virtual_machine *v=vm;
					f->gc_acquire();
					//v->execution_stack->pop_frame(vm->execution_stack->peek_frame());
					show_in_exception_stack(false);
					finalized(true);
					v->raise_too_many_parameters(f);
					f->gc_release(*vm);
					return executionstackreturnvalue::FUNCTION_CALL;
				}
			}
			else {
				exec_class_object *p=vm->data_from_object<exec_class_object>(slf.get_object());

				if (p->operators[E_NEW]) {
					RCASSERT(vm->execution_stack->peek_frame()==this);
					if (vm->prepare_call_function(return_value,*p->operators[E_NEW],cp,NULL)) {
						vm->execution_stack->peek_frame()->return_handling_mode=vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_CANT_BE_UNUSED;
					}

					show_in_exception_stack(false);
					finalized(true);

					return executionstackreturnvalue::FUNCTION_CALL;
				}
				else if (!p->constructable) {
					vm->raise_unsupported_operation();
					return executionstackreturnvalue::FUNCTION_CALL;
				}
				else {
					exec_object *o=vm->allocate_object(slf.get_object(),0);

					if (o->type()->CO().operators[E_INIT]) {
						exec_variable v,fnc;
						v.set_object(o);
						RCASSERT(v.lookup_operator(fnc,*vm,E_INIT));

						//vm_execution_stack_elem_base *prevsf=vm->execution_stack->peek_frame();
						//prevsf->gc_acquire();
						//vm->execution_stack->pop_frame(prevsf);

						if (vm->prepare_call_function(return_value,fnc,cp,NULL)) {
							//vm->execution_stack->peek_frame()->return_handling_mode=return_handling_mode;
							//return_handling_mode=vm_execution_stack_elem_base::RETURN_HANDLING_NONE;

							if (return_handling_mode==vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION) {
								vm->execution_stack->peek_frame()->return_handling_mode=RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION;
							}
							else vm->execution_stack->peek_frame()->return_handling_mode=RETURN_HANDLING_OPERATOR_RETURN_INIT;
							return_handling_mode=vm_execution_stack_elem_base::RETURN_HANDLING_NONE;
							vm->execution_stack->peek_frame()->init_object=o;
						}
						else {
							o->gc_release(*vm);
							RCASSERT(vm->execution_stack->peek_frame()->return_handling_mode==RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION);
						}

						//RCASSERT(return_handling_mode==vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION ||
						//	return_handling_mode==vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT);

						//if (vm->prepare_call_function(return_value,fnc,cp,NULL)) {
						//	if (prevsf->return_handling_mode==vm_execution_stack_elem_base::RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION) {
						//		vm->execution_stack->peek_frame()->return_handling_mode=RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION;
						//	}
						//	else vm->execution_stack->peek_frame()->return_handling_mode=RETURN_HANDLING_OPERATOR_RETURN_INIT;
						//	vm->execution_stack->peek_frame()->init_object=o;
						//}
						//else o->gc_release(*vm);
						//prevsf->gc_release(*vm);

						fnc.gc_release(*vm);
						show_in_exception_stack(false);
						finalized(true);
						return executionstackreturnvalue::FUNCTION_CALL;
					}
					else {
						if (cp.normal_params_count+cp.list_params_count>0 || (cp.map && cp.map->map.size()>0)) {
							vm->raise_no_constructor();
							o->gc_release(*vm);
							return executionstackreturnvalue::FUNCTION_CALL;
						}
						return_value->set_object(o);
						return executionstackreturnvalue::RETURN;
					}
				}
			}
			RCASSERT(0);
		}
	D_END

	D_SELF0(class,str,exec_class_object*)
	{
		return_value->set_string(vm->allocate_string("type "+self->name->str()));
		return executionstackreturnvalue::RETURN;
	}
	D_END

	D_SELF0(class,bool,exec_class_object*)
	{
		return_value->set_bool(true);
		return executionstackreturnvalue::RETURN;
	}
	D_END

	D_SELF1(class,eq,exec_class_object*,exec_class_object*)
	{
		return_value->set_bool(self==p1);
		return executionstackreturnvalue::RETURN;
	}
	D_END

	D_SELF1(class,noteq,exec_class_object*,exec_class_object*)
	{
		return_value->set_bool(self!=p1);
		return executionstackreturnvalue::RETURN;
	}
	D_END

	D_SELF0(class,hash,exec_class_object*)
	{
		return_value->set_int(reinterpret_cast<owca_int>(self));
		return executionstackreturnvalue::RETURN;
	}
	D_END

	void virtual_machine::initialize_class(internal_class &c)
	{
		c._setinheritable(false);

		M_OPER0(c,class,str,(*this,"$str"));
		M_OPER0(c,class,bool,(*this,"$bool"));
		M_OPER_(c,class,call,(*this,"$call"));
		M_OPER0(c,class,hash,(*this,"$hash"))
		M_OPER1(c,class,eq,(*this,"$eq","other"))
		M_OPER1(c,class,noteq,(*this,"$noteq","other"))

		c._setstructure<exec_class_object>();
	}
} }












