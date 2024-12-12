#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_bool_validate(opcode_validator &oe)
	{
		return oe.get_operands(1);
	}

	RCLMFUNCTION bool op_bool_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=1);
		exec_variable *params=&oe.temp(oe.tempstackactpos-1);
		exec_variable &retval=params[0];

		bool b;
		if (oe.vm->calculate_bool_nocall(b,*params)) {
			params[0].gc_release(*oe.vm);
			params[0].set_bool(b);
			return true;
		}
		oe.prepare_call_operator_stack(E_BOOL);
		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		return false;
	}

} }


