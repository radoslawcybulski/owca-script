#include "stdafx.h"
#include "base.h"
#include "virtualmachine.h"
#include "exec_class_int.h"
#include "vm_execution_stack_elem_external.h"

namespace owca { namespace __owca__ {

	D_FUNC0(null,new)
		{
			return_value->reset();
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(null,str,null)
		{
			return_value->set_string(vm->allocate_string("null"));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(null,bool,null)
		{
			return_value->set_bool(false);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF0(null,hash,null)
		{
			return_value->set_int(reinterpret_cast<owca_int>(vm->class_null));
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(null,eq,null,null)
		{
			return_value->set_bool(true);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	D_SELF1(null,noteq,null,null)
		{
			return_value->set_bool(false);
			return executionstackreturnvalue::RETURN;
		}
	D_END

	void virtual_machine::initialize_null(internal_class &c)
	{
		c._setinheritable(false);
		//c._setconstructable(false);

		M_OPER0(c,null,new,(*this,"$new"));
		M_OPER0(c,null,str,(*this,"$str"));
		M_OPER0(c,null,bool,(*this,"$bool"));
		M_OPER0(c,null,hash,(*this,"$hash"));
		M_OPER1(c,null,eq,(*this,"$eq","other"));
		M_OPER1(c,null,noteq,(*this,"$noteq","other"));
	}
} }












