#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_if.h"
#include "virtualmachine.h"
#include "exec_base.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_if_validate(opcode_validator &oe)
	{
		opcode_executer_jump elseblock,finallyblock,done;
		bool b1,b2,b3;

		b1=false;
		b2=b3=true;
		if (!oe.push_local_stack_data_size(sizeof(op_flow_if))) return false;
		if (!oe.get(elseblock) || !oe.get(finallyblock) || !oe.get(done)) return false;
		unsigned int count;
		if (!oe.get(count) || count == 0) return false;
		while(count-- > 0) {
			opcode_executer_jump jmp1;
			if (!oe.get(jmp1)) return false;
			if (oe.temporary_variables_count()!=0) return false;
			if (!oe.validate_read_expr()) return false;
			if (oe.temporary_variables_count()!=0) return false;
			if (!oe.validate_flow()) return false;
			if (oe.execution_can_continue) b1=true;
			if (oe.compare(jmp1)!=0) return false;
		}
		if (elseblock != done) {
			if (oe.compare(elseblock)!=0) return false;
			if (oe.temporary_variables_count()!=0) return false;
			if (!oe.validate_flow()) return false;
			b2=oe.execution_can_continue;
		}
		if (finallyblock != done) {
			if (oe.compare(finallyblock)!=0) return false;
			if (oe.temporary_variables_count()!=0) return false;
			if (!oe.validate_flow()) return false;
			b3=oe.execution_can_continue;
		}
		if (oe.compare(done)!=0) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_if))) return false;
		oe.execution_can_continue=oe.continue_opcodes=!((!b2 && !b3) || (!b1 && !b2));
		return true;
	}

	bool op_flow_if::set_dest_else_finally(vm_execution_stack_elem_internal &oe, vm_execution_stack_elem_internal_jump j)
	{
		if (j == done) {
			oe.set_code_position(done);
			return false;
		}
		oe.set_code_position(j);
		mode=2;
		return true;
	}

	bool op_flow_if::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		oe.r=returnvalueflow::CONTINUE_OPCODES;
		switch(mode) {
		case 0: {
			// if blocks
			RCASSERT(oe.tempstackactpos==1);
			bool b=oe.temp(0).get_bool();
			oe.tempstackactpos=0;
			if (b) {
				mode=1;
				return true;
			}
			oe.set_code_position(next);
			if (--count == 0) {
				// all ifs are done, go to else block
				return set_dest_else_finally(oe, elseblock);
			}
			oe >> next;
			return true; }
		case 1:
			return set_dest_else_finally(oe,finallyblock);
		case 2:
			oe.set_code_position(done);
			return false;
		default:
			RCASSERT(0);
			return false;
		}
	}

	RCLMFUNCTION bool op_if_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_if *ff;
		oe.push(ff,0);

		ff->mode=0;

		RCASSERT(oe.tempstackactpos==0);

		oe >> ff->elseblock >> ff->finallyblock >> ff->done >> ff->count;
		oe >> ff->next;

		return true;
	}

} }












