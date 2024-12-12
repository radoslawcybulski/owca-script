#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "exec_string.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_add_validate(opcode_validator &oe)
	{
		return oe.get_operands(2);
	}

	owca_internal_string *string_add(virtual_machine *vm, owca_internal_string *s1, owca_internal_string *s2);

	RCLMFUNCTION bool op_add_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=2);
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);
		exec_variable &retval=params[0];

		switch(params[1].mode()==VAR_OBJECT ? VAR_OBJECT : params[0].mode()) {
		case VAR_INT:
			switch(params[1].mode()) {
			case VAR_INT:
				retval.set_int(params[0].get_int()+params[1].get_int());
				break;
			case VAR_REAL:
				retval.set_real(params[0].get_int()+params[1].get_real());
				break;
			default:
				goto cont;
			}
			break;
		case VAR_REAL:
			switch(params[1].mode()) {
			case VAR_INT:
				retval.set_real(params[0].get_real()+params[1].get_int());
				break;
			case VAR_REAL:
				retval.set_real(params[0].get_real()+params[1].get_real());
				break;
			default:
				goto cont;
			}
			break;
		case VAR_STRING:
			switch(params[1].mode()) {
			case VAR_STRING: {
				owca_internal_string *s=string_add(oe.vm,params[0].get_string(),params[1].get_string());
				params[0].gc_release(*oe.vm);
				params[1].gc_release(*oe.vm);
				params[0].set_string(s);
				break; }
			default:
				goto cont;
			}
			break;
		default:
cont:
			oe.prepare_call_operator_stack(E_ADD);
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
			return false;
		}
		--oe.tempstackactpos;
		return true;
	}

} }


