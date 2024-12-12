#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "exec_function_stack_data.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_yield_validate(opcode_validator &oe)
	{
		if (oe.temporary_variables_count()!=1) return false;
		oe.pop_temporary_variable();
		return true;
	}

	RCLMFUNCTION bool op_yield_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos==1);
		*oe.return_value=oe.temp(0);
		oe.tempstackactpos=0;
		oe.r=returnvalueflow::YIELD;
		return false;
	}

} }


