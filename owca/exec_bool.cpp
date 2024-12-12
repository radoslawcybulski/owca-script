#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_class_int.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_external.h"

namespace owca { namespace __owca__ {
	D_FUNC1(bool,new,const exec_variable *)
		{
			switch(mode) {
			CASE(0)
				if (p1->mode()==VAR_NO_PARAM_GIVEN) {
					return_value->set_bool(false);
					break;
				}
				CALC(1,vm->calculate_bool(return_value,*p1));
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

	D_SELF0(bool,str,bool)
		{
			return_value->set_string(vm->allocate_string(self ? "true" : "false"));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(bool,bool,bool)
		{
			return_value->set_bool(self);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(bool,eq,bool,bool)
		{
			return_value->set_bool(self==p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(bool,noteq,bool,bool)
		{
			return_value->set_bool(self!=p1);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(bool,hash,bool)
		{
			return_value->set_int(self ? reinterpret_cast<owca_int>(vm->class_bool) : ~reinterpret_cast<owca_int>(vm->class_bool));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_bool(internal_class &c)
	{
		c._setinheritable(false);

		M_OPER1(c,bool,new,(*this,"$new","value",dv_no_param_given));
		M_OPER0(c,bool,str,(*this,"$str"));
		M_OPER0(c,bool,hash,(*this,"$hash"));
		M_OPER0(c,bool,bool,(*this,"$bool"));

		M_OPER1(c,bool,eq,(*this,"$eq","other"));
		M_OPER1(c,bool,noteq,(*this,"$noteq","other"));
	}
} }


