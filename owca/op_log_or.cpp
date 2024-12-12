#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_log_or.h"
#include "returnvalue.h"
#include "virtualmachine.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_log_or_validate(opcode_validator &oe)
	{
		opcode_executer_jump jmp;
		if (!oe.push_local_stack_data_size(sizeof(op_flow_log_or))) return false;
		if (!oe.get(jmp)) return false;
		if (!oe.pop_temporary_variable()) return false;
		if (!oe.validate_read_expr()) return false;
		if (oe.compare(jmp)!=0) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_log_or))) return false;
		if (!oe.push_temporary_variable()) return false;
		oe.execution_can_continue=oe.continue_opcodes=true;
		return true;
	}

	bool op_flow_log_or::update_bool_value(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &v=oe.temp(oe.tempstackactpos-1);
		bool b=oe.vm->calculate_bool(&tmp,v);
		v.gc_release(*oe.vm);
		--oe.tempstackactpos;
		return b;
	}

	bool op_flow_log_or::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		switch(mode) {
		case 0:
			if (update_bool_value(oe)) {
				// $bool call
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				mode=1;
				return true;
			}
		case 1:
			oe.r=returnvalueflow::CONTINUE_OPCODES;
			if (tmp.get_bool()) {
				exec_variable &v=oe.temp(oe.tempstackactpos++);
				v.set_bool(true);
				oe.set_code_position(jmp);
				return false;
			}
			mode=2;
			return true;
		case 2:
			if (update_bool_value(oe)) {
				// $bool call
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				mode=3;
				return true;
			}
		case 3: {
			oe.r=returnvalueflow::CONTINUE_OPCODES;
			exec_variable &v=oe.temp(oe.tempstackactpos++);
			v.set_bool(tmp.get_bool());
			return false; }
		}
		RCASSERT(0);
		return false;
	}

	RCLMFUNCTION bool op_log_or_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_log_or *ff;

		oe.push(ff,0);

		oe >> ff->jmp;
		ff->mode=0;

		if (!ff->resume_fin(oe)) oe.pop(ff);
		return oe.r.type()==returnvalueflow::CONTINUE_OPCODES;
	}

} }


