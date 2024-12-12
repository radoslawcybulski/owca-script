#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_mod_validate(opcode_validator &oe)
	{
		return oe.get_operands(2);
	}

	RCLMFUNCTION bool op_mod_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=2);
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);
		exec_variable &retval=params[0];

		owca_int i1,i2;
		if (params[0].get_int(i1) && params[1].get_int(i2)) {
			if (i2==0) {
				oe.vm->raise_division_by_zero();
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
			}
			else {
				retval.set_int(i1%i2);
				--oe.tempstackactpos;
				return true;
			}
		}
		else {
			oe.prepare_call_operator_stack(E_MOD);
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		}
		return false;
	}

} }












