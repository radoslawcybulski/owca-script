#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "op_validate.h"
#include "op_execute.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_noop_validate(opcode_validator &oe)
	{
		return true;
	}

	RCLMFUNCTION bool op_noop_flow(vm_execution_stack_elem_internal &oe)
	{
		return true;
	}

} }


