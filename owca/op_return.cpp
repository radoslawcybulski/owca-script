#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_return_validate(opcode_validator &oe)
	{
		if (oe.temporary_variables_count()!=1) return false;
		oe.pop_temporary_variable();
		oe.execution_can_continue=oe.continue_opcodes=false;
		return true;
	}

	RCLMFUNCTION bool op_return_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos==1);
		*oe.return_value=oe.temp(0);
		oe.tempstackactpos=0;
		oe.r=returnvalueflow::RETURN;
		return false;
	}

	opcode_validator::boolean_result op_return_no_value_validate(opcode_validator &oe)
	{
		if (oe.temporary_variables_count()!=0) return false;
		oe.execution_can_continue=oe.continue_opcodes=false;
		return true;
	}

	RCLMFUNCTION bool op_return_no_value_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos==0);
		oe.r=returnvalueflow::RETURN_NO_VALUE;
		return false;
	}

} }


