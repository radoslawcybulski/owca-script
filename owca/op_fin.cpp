#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "vm_execution_stack_elem_internal.h"
#include "returnvalueflow.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_fin_validate(opcode_validator &oe)
	{
		oe.execution_can_continue=true;
		oe.continue_opcodes=false;
		return true;
	}

	RCLMFUNCTION bool op_fin_flow(vm_execution_stack_elem_internal &oe)
	{
		oe.r=returnvalueflow::FIN;
		return false;
	}

	opcode_validator::boolean_result op_fin_1_clear_validate(opcode_validator &oe)
	{
		if (oe.temporary_variables_count()!=1) return false;
		oe.pop_temporary_variable();
		return true;
	}

	RCLMFUNCTION bool op_fin_1_clear_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos==1);
		oe.temp(0).gc_release(*oe.vm);
		oe.tempstackactpos=0;
		return true;
	}

	opcode_validator::boolean_result op_fin_1_clear_for_validate(opcode_validator &oe)
	{
		if (oe.temporary_variables_count()!=1) return false;
		oe.pop_temporary_variable();
		oe.execution_can_continue=true;
		oe.continue_opcodes=false;
		return true;
	}

	RCLMFUNCTION bool op_fin_1_clear_for_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos==1);
		oe.temp(0).gc_release(*oe.vm);
		oe.tempstackactpos=0;
		return true;
	}

	//opcode_validator::boolean_result op_fin_ignore_validate(opcode_validator &oe)
	//{
	//	return true;
	//}

	//RCLMFUNCTION bool op_fin_ignore_flow(vm_execution_stack_elem_internal &oe)
	//{
	//	oe.r=returnvalueflow::FIN;
	//	return false;
	//}

} }


