#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_raise_validate(opcode_validator &oe)
	{
		if (oe.temporary_variables_count()!=1) return false;
		oe.pop_temporary_variable();
		oe.execution_can_continue=oe.continue_opcodes=false;
		return true;
	}

	RCLMFUNCTION bool op_raise_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos==1);
		exec_variable &v=oe.temp(oe.tempstackactpos-1);
		if (v.mode()!=VAR_OBJECT || !v.type(oe.vm->class_exception)) {
			oe.vm->raise_invalid_param(v,oe.vm->class_exception);
			oe.r=oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		}
		else {
			if (oe.vm->execution_exception_object_thrown) oe.vm->execution_exception_object_thrown->gc_release(*oe.vm);
			if (oe.exception_object_being_handled) {
				oe.exception_object_being_handled->gc_release(*oe.vm);
				oe.exception_object_being_handled=NULL;
			}
			oe.vm->execution_exception_object_thrown=v.get_object();
			oe.tempstackactpos=0;
			oe.r=returnvalueflow::EXCEPTION;
		}
		return false;
	}

	opcode_validator::boolean_result op_raise_no_value_validate(opcode_validator &oe)
	{
		if (oe.temporary_variables_count()!=0) return false;
		oe.execution_can_continue=oe.continue_opcodes=false;
		return true;
	}

	RCLMFUNCTION bool op_raise_no_value_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos==0);
		RCASSERT(oe.exception_object_being_handled!=NULL);
		RCASSERT(oe.vm->execution_exception_object_thrown==NULL);
		oe.vm->execution_exception_object_thrown=oe.exception_object_being_handled;
		oe.exception_object_being_handled=NULL;
		oe.r=returnvalueflow::EXCEPTION;
		RCASSERT(oe.tempstackactpos==0);
		return false;
	}

} }


