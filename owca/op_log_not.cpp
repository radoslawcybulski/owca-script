#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_log_not.h"
#include "returnvalue.h"
#include "virtualmachine.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_log_not_validate(opcode_validator &oe)
	{
		if (!oe.push_local_stack_data_size(sizeof(op_flow_log_not))) return false;
		if (!oe.get_operands(1)) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_log_not))) return false;
		return true;
	}

	void op_flow_log_not::_mark_gc(const gc_iteration &gc) const
	{
		gc_iteration::debug_info _d("op_flow_log_not: tmp");
		tmp.gc_mark(gc);
	}

	void op_flow_log_not::_release_resources(virtual_machine &vm)
	{
		tmp.gc_release(vm);
	}

	bool op_flow_log_not::update_bool_value(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &v=oe.temp(oe.tempstackactpos-1);
		bool b=oe.vm->calculate_bool(&tmp,v);
		v.gc_release(*oe.vm);
		--oe.tempstackactpos;
		return b;
	}

	bool op_flow_log_not::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		switch(mode++) {
		case 0:
			if (update_bool_value(oe)) {
				// $bool call
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				tmp.reset();
				return true;
			}
		case 1:
			oe.r=returnvalueflow::CONTINUE_OPCODES;
			exec_variable &v=oe.temp(oe.tempstackactpos++);
			v.set_bool(!tmp.get_bool());
			return false;
		}
		RCASSERT(0);
		return false;
	}

	RCLMFUNCTION bool op_log_not_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_log_not *ff;

		oe.push(ff,0);

		ff->mode=0;

		if (!ff->resume_fin(oe)) oe.pop(ff);
		return oe.r.type()==returnvalueflow::CONTINUE_OPCODES;
	}

} }


