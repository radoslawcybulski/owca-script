#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_is_validate(opcode_validator &oe)
	{
		return oe.get_operands(2);
	}

	RCLMFUNCTION bool op_is_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=2);
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);

		bool is_same=params[0].is_same(params[1]);
		params[0].gc_release(*oe.vm);
		params[1].gc_release(*oe.vm);
		--oe.tempstackactpos;
		params[0].set_bool(is_same);

		return true;
	}

} }


