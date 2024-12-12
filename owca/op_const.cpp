#include "stdafx.h"
#include "debug_strings.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "exec_string.h"
#include "returnvalue.h"
#include "exec_variablelocation.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_create_int_validate(opcode_validator &oe)
	{
		if (!oe.push_temporary_variable()) return false;
		owca_int v;
		return oe.get(v);
	}

	RCLMFUNCTION bool op_create_int_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &rv=oe.temp(oe.tempstackactpos++);

		owca_int v;
		oe >> v;
		rv.set_int(v);
		return true;
	}

	opcode_validator::boolean_result op_create_real_validate(opcode_validator &oe)
	{
		if (!oe.push_temporary_variable()) return false;
		owca_real v;
		return oe.get(v);
	}

	RCLMFUNCTION bool op_create_real_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &rv=oe.temp(oe.tempstackactpos++);

		owca_real v;
		oe >> v;
		rv.set_real(v);
		return true;
	}

	opcode_validator::boolean_result op_create_string_validate(opcode_validator &oe)
	{
		if (!oe.push_temporary_variable()) return false;
		owca_internal_string *v=NULL;
		return oe.get(v);
	}

	RCLMFUNCTION bool op_create_string_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &rv=oe.temp(oe.tempstackactpos++);

		owca_internal_string *v=NULL;
		oe >> v;
		rv.set_string(v);
        v->gc_acquire();
//#ifdef RCDEBUG_DEBUG_STRINGS
//		rv.set_string(oe.vm->allocate_string(v->data_pointer(),v->data_size(),v->character_count()));
//#else
//		rv.set_string(v);
//		v->gc_acquire();
//#endif
		return true;
	}

	opcode_validator::boolean_result op_create_null_validate(opcode_validator &oe)
	{
		if (!oe.push_temporary_variable()) return false;
		return true;
	}

	RCLMFUNCTION bool op_create_null_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &rv=oe.temp(oe.tempstackactpos++);
		rv.set_null();
		return true;
	}

	opcode_validator::boolean_result op_create_true_validate(opcode_validator &oe)
	{
		if (!oe.push_temporary_variable()) return false;
		return true;
	}

	RCLMFUNCTION bool op_create_true_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &rv=oe.temp(oe.tempstackactpos++);
		rv.set_bool(true);
		return true;
	}

	opcode_validator::boolean_result op_create_false_validate(opcode_validator &oe)
	{
		if (!oe.push_temporary_variable()) return false;
		return true;
	}

	RCLMFUNCTION bool op_create_false_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &rv=oe.temp(oe.tempstackactpos++);
		rv.set_bool(false);
		return true;
	}

} }












