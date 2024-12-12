#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "returnvalue.h"
#include "virtualmachine.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_sign_change_validate(opcode_validator &oe)
	{
		return oe.get_operands(1);
	}

	RCLMFUNCTION bool op_sign_change_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=1);
		exec_variable *params=&oe.temp(oe.tempstackactpos-1);
		exec_variable &retval=params[0];

		switch(params[0].mode()) {
		case VAR_INT:
			retval.set_int(-params[0].get_int());
			break;
		case VAR_REAL:
			retval.set_real(-params[0].get_real());
			break;
		case VAR_NULL:
		case VAR_BOOL:
		case VAR_STRING:
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_NAMESPACE:
		case VAR_WEAK_REF:
		case VAR_OBJECT:
			oe.prepare_call_operator_stack(E_SIGN_CHANGE);
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
			return false;
		default:
			RCASSERT(0);
		}
		return true;
	}

} }


