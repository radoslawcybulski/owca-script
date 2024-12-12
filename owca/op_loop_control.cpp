#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "exec_variablelocation.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_break_validate(opcode_validator &oe)
	{
		exec_variable_location vl;
		if (!oe.get(vl) || (vl.valid() && !oe.check(vl))) return false;
		oe.execution_can_continue=oe.continue_opcodes=false;
		return true;
	}

	opcode_validator::boolean_result op_restart_validate(opcode_validator &oe)
	{
		exec_variable_location vl;
		if (!oe.get(vl) || (vl.valid() && !oe.check(vl))) return false;
		oe.execution_can_continue=oe.continue_opcodes=false;
		return true;
	}

	opcode_validator::boolean_result op_finally_validate(opcode_validator &oe)
	{
		exec_variable_location vl;
		if (!oe.get(vl) || (vl.valid() && !oe.check(vl))) return false;
		oe.execution_can_continue=oe.continue_opcodes=false;
		return true;
	}

	opcode_validator::boolean_result op_continue_validate(opcode_validator &oe)
	{
		exec_variable_location vl;
		if (!oe.get(vl) || (vl.valid() && !oe.check(vl))) return false;
		oe.execution_can_continue=oe.continue_opcodes=false;
		return true;
	}

	bool op_break_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable_location v;
		oe >> v;
		oe.r=returnvalueflow(returnvalueflow::LOOP_CONTROL,returnvalueflow::LP_BREAK,v.offset());
		return false;
	}

	bool op_continue_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable_location v;
		oe >> v;
		oe.r=returnvalueflow(returnvalueflow::LOOP_CONTROL,returnvalueflow::LP_CONTINUE,v.offset());
		return false;
	}

	bool op_restart_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable_location v;
		oe >> v;
		oe.r=returnvalueflow(returnvalueflow::LOOP_CONTROL,returnvalueflow::LP_RESTART,v.offset());
		return false;
	}

	bool op_finally_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable_location v;
		oe >> v;
		oe.r=returnvalueflow(returnvalueflow::LOOP_CONTROL,returnvalueflow::LP_FINALLY,v.offset());
		return false;
	}

} }


