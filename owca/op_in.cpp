#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_in_validate(opcode_validator &oe)
	{
		return oe.get_operands(2);
	}

	RCLMFUNCTION bool op_in_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=2);
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);

		std::swap(params[0],params[1]);
		oe.prepare_call_operator_stack(E_IN);
		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		return false;
	}

} }


