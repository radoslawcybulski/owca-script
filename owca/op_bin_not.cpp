#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_bin_not_validate(opcode_validator &oe)
	{
		return oe.get_operands(1);
	}

	RCLMFUNCTION bool op_bin_not_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=1);
		exec_variable *params=&oe.temp(oe.tempstackactpos-1);
		exec_variable &retval=params[0];

		owca_int i1;
		if (params[0].get_int(i1)) {
			retval.set_int(~i1);
			return true;
		}
		oe.prepare_call_operator_stack(E_BIN_NOT);
		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		return false;
	}

} }


