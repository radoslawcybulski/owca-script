#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_assign_tuple.h"
#include "returnvalue.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "exec_tuple_object.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_assign_tuple_validate(opcode_validator &oe)
	{
		unsigned int cnt;
		if (!oe.get(cnt) || cnt==0) return false;
		if (!oe.push_local_stack_data_size(sizeof(op_flow_assign_tuple))) return false;
		for(unsigned int i=0;i<cnt;++i) {
			if (!oe.validate_write_expr_assign_tuple()) return false;
			//if (!oe.pop_temporary_variable()) return false;
		}
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_assign_tuple))) return false;
		oe.execution_can_continue=oe.continue_opcodes=true;
		//if (!oe.push_temporary_variable()) return false;
		return true;
	}

	bool op_flow_assign_tuple::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &retval=oe.temp(oe.tempstackactpos-1);

		switch(mode) {
		case 2:
			retval.gc_release(*oe.vm);
			retval.reset();
		case 0:
			mode=1;
			if (oe.vm->calculate_iter_next(&retval,generator)) {
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return true;
			}
		case 1:
			if (retval.is_no_return_value()) {
				// generator is done

				if (index<count) {
					oe.vm->raise_too_little_iter_obj();
					oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
				}
				else {
					RCASSERT(index==count);
					oe.r=returnvalueflow::CONTINUE_OPCODES;
				}
				return false;
			}
			if (index>=count) {
				oe.vm->raise_too_much_iter_obj();
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
				return false;
			}
			++index;
			oe.r=returnvalueflow::CONTINUE_OPCODES;
			mode=2;
			return true;
		default:
			RCASSERT(0);
			return false;
		}
	}

	void op_flow_assign_tuple::_mark_gc(const gc_iteration &gc) const
	{
		gc_iteration::debug_info _d("op_flow_assign_tuple: generator variable");
		generator.gc_mark(gc);
	}

	void op_flow_assign_tuple::_release_resources(virtual_machine &vm)
	{
		generator.gc_release(vm);
	}

	RCLMFUNCTION bool op_assign_tuple_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_assign_tuple *ff;

		oe.push(ff,0);
		ff->index=0;
		oe >> ff->count;

		ff->mode=0;
		ff->generator.reset();

		RCASSERT(ff->count>0);

		exec_variable *params=&oe.temp(oe.tempstackactpos-1);
		if (oe.vm->calculate_generator(&ff->generator,params[0])) {
			params[0].gc_release(*oe.vm);
			params[0].reset();
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
			return false;
		}
		params[0].gc_release(*oe.vm);
		params[0].reset();

		return !ff->resume_fin(oe);
	}

} }


