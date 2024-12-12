#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_while.h"
#include "virtualmachine.h"
#include "exec_stack.h"
#include "exec_base.h"
#include "returnvalue.h"
#include "exec_variablelocation.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_while_validate(opcode_validator &oe)
	{
		opcode_executer_jump mainblock,elseblock,finallyblock,done;
		exec_variable_location vl;

		if (oe.temporary_variables_count()!=0) return false;
		if (!oe.push_local_stack_data_size(sizeof(op_flow_while))) return false;
		if (!oe.get(mainblock) || !oe.get(elseblock) || !oe.get(finallyblock) || !oe.get(done) || !oe.get(vl) || (vl.valid() && !oe.check(vl))) return false;
		if (!oe.validate_read_expr()) return false;
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
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_while))) return false;
		oe.execution_can_continue=oe.continue_opcodes=true;
		return true;
	}

	bool op_flow_while::set_dest_else_finally(vm_execution_stack_elem_internal &oe)
	{
		vm_execution_stack_elem_internal_jump j=firsttime ? elseblock : finallyblock;
		if (j == done) {
			oe.set_code_position(done);
			return false;
		}
		oe.set_code_position(j);
		mode=2;
		return true;
	}

	RCLMFUNCTION bool op_flow_while::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		oe.r=returnvalueflow::CONTINUE_OPCODES;
		switch(mode) {
		case 0: {
			RCASSERT(oe.tempstackactpos==1);
			exec_variable &v=oe.temp(0);
			oe.tempstackactpos=0;
			RCASSERT(v.mode()==VAR_BOOL); // no need to dereference bool value
			if (!v.get_bool()) {
				return set_dest_else_finally(oe);
			}
			else {
				firsttime=0;
				mode=1;
				RCASSERT(oe.actual_code_position()==mainblock);
			}
			break; }
		case 1:
			oe.set_code_position(condition);
			++countervalue;
			if (countervar) countervar->set_int(countervalue);
			mode=0;
			break;
		case 2:
			oe.set_code_position(done);
			return false;
		default:
			RCASSERT(0);
		}
		return true;
	}

	RCLMFUNCTION bool op_flow_while::resume_loop_control(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(mode==1);
		if (oe.r.data()!=exec_variable_location::_INVALID && oe.r.data()!=counter.offset()) return false;
		unsigned char rmode=oe.r.mode();

		oe.r=returnvalueflow::CONTINUE_OPCODES;
		switch(rmode) {
		case returnvalueflow::LP_BREAK:
			oe.set_code_position(done);
			return false;
		case returnvalueflow::LP_CONTINUE:
			oe.set_code_position(condition);
			++countervalue;
			if (countervar) countervar->set_int(countervalue);
			mode=0;
			return true;
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

	RCLMFUNCTION bool op_while_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_while *ff;
		oe.push(ff,0);

		RCASSERT(oe.tempstackactpos==0);

		oe >> ff->mainblock >> ff->elseblock >> ff->finallyblock >> ff->done >> ff->counter;
		ff->countervar=ff->counter.valid() ? &oe.get(ff->counter) : NULL;
		ff->countervalue=0;

		if (ff->countervar) {
			ff->countervar->gc_release(*oe.vm);
			ff->countervar->set_int(0);
		}
		ff->firsttime=1;
		ff->condition=oe.actual_code_position();
		ff->mode=0;
		return true;
	}

} }












