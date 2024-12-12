#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_for.h"
#include "virtualmachine.h"
#include "exec_stack.h"
#include "exec_base.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	void op_flow_for::_mark_gc(const gc_iteration &gc) const
	{
		{
			gc_iteration::debug_info _d("op_flow_for: generator variable");
			generator.gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_for: tmp");
			temp.gc_mark(gc);
		}
	}

	void op_flow_for::_release_resources(virtual_machine &vm)
	{
		generator.gc_release(vm);
		temp.gc_release(vm);
	}

	opcode_validator::boolean_result op_for_validate(opcode_validator &oe)
	{
		opcode_executer_jump mainblock,elseblock,finallyblock,done;
		exec_variable_location vl;

		if (oe.temporary_variables_count()!=0) return false;
		if (!oe.push_local_stack_data_size(sizeof(op_flow_for))) return false;
		if (!oe.get(mainblock) || !oe.get(elseblock) || !oe.get(finallyblock) || !oe.get(done) || !oe.get(vl) || (vl.valid() && !oe.check(vl))) return false;

		if (!oe.validate_read_expr()) return false;
		if (!oe.push_temporary_variable()) return false;
		if (!oe.validate_write_expr_for()) return false;

		if (oe.compare(mainblock)!=0) return false;
		if (!oe.validate_flow()) return false;
		if (elseblock != done) {
			if (oe.compare(elseblock)!=0) return false;
			if (!oe.validate_flow()) return false;
		}
		if (finallyblock != done) {
			if (oe.compare(finallyblock)!=0) return false;
			if (!oe.validate_flow()) return false;
		}
		if (oe.compare(done)!=0) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_for))) return false;
		oe.execution_can_continue=oe.continue_opcodes=true;
		return true;
	}

	bool op_flow_for::set_dest_else_finally(vm_execution_stack_elem_internal &oe)
	{
		vm_execution_stack_elem_internal_jump j=firsttime ? elseblock : finallyblock;
		if (j == done) {
			oe.set_code_position(done);
			return false;
		}
		oe.set_code_position(j);
		mode=20;
		return true;
	}

	RCLMFUNCTION bool op_flow_for::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		oe.r=returnvalueflow::CONTINUE_OPCODES;
		switch(mode) {
		case 0:
			RCASSERT(oe.tempstackactpos==1);
			generator=oe.temp(0);
			--oe.tempstackactpos;
			mode=10;
			begin=oe.actual_code_position();
		case 10:
			++countervalue;
			if (countervar) countervar->set_int(countervalue);

			oe.vm->prepare_call_function(&temp,generator,NULL,0);
			mode=11;
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
			oe.set_code_position(begin);
			break;
		case 11:
			if (oe.vm->execution_no_return_value) { // done
				return set_dest_else_finally(oe);
			}
			RCASSERT(oe.tempstackactpos==0);
			oe.temp(oe.tempstackactpos++)=temp;
			temp.reset();
			mode=10;
			firsttime=0;
			RCASSERT(oe.actual_code_position()==begin);
			break;
		case 20:
			oe.set_code_position(done);
			return false;
		default:
			RCASSERT(0);
		}
		return true;
	}

	RCLMFUNCTION bool op_flow_for::resume_loop_control(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(mode==10);
		if (oe.r.data()!=exec_variable_location::_INVALID && oe.r.data()!=counter.offset()) return false;
		unsigned char rmode=oe.r.mode();

		oe.r=returnvalueflow::CONTINUE_OPCODES;
		switch(rmode) {
		case returnvalueflow::LP_BREAK:
			oe.set_code_position(done);
			return false;
		case returnvalueflow::LP_CONTINUE:
			mode=10;
			return op_flow_for::resume_fin(oe);
		case returnvalueflow::LP_FINALLY:
			return set_dest_else_finally(oe);
		case returnvalueflow::LP_RESTART:
			oe.set_code_position(mainblock);
			return true;
		default:
			RCASSERT(0);
			return false;
		}
	}

	RCLMFUNCTION bool op_for_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_for *ff;
		oe.push(ff,0);

		RCASSERT(oe.tempstackactpos==0);

		oe >> ff->mainblock >> ff->elseblock >> ff->finallyblock >> ff->done >> ff->counter;
		ff->countervar=ff->counter.valid() ? &oe.get(ff->counter) : NULL;
		ff->countervalue=-1;

		if (ff->countervar) {
			ff->countervar->gc_release(*oe.vm);
			ff->countervar->set_int(0);
		}

		ff->firsttime=1;
		ff->generator.reset();
		ff->temp.reset();

		ff->mode=0;
		return true;
	}

} }


