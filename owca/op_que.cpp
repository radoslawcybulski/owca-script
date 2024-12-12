#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_que.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_que_validate(opcode_validator &oe)
	{
		if (!oe.push_local_stack_data_size(sizeof(op_flow_que))) return false;
		if (!oe.pop_temporary_variable()) return false;
		unsigned int read;
		opcode_executer_jump jmp1,jmp2;
		if (!oe.get(read) || (read!=0 && read!=1) || !oe.get(jmp1) || !oe.get(jmp2)) return false;
		if (read) {
			if (!oe.validate_read_expr()) return false;
			if (oe.compare(jmp1)!=0) return false;
			if (!oe.validate_read_expr()) return false;
			if (oe.compare(jmp2)!=0) return false;
			if (!oe.push_temporary_variable()) return false;
		}
		else {
			if (!oe.validate_write_expr_que()) return false;
			if (oe.compare(jmp1)!=0) return false;
			if (!oe.validate_write_expr_que()) return false;
			if (oe.compare(jmp2)!=0) return false;
		}
		oe.execution_can_continue=oe.continue_opcodes=true;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_que))) return false;
		return true;
	}

	void op_flow_que::update_opcodes(vm_execution_stack_elem_internal &oe, bool b)
	{
		if (b) {
			mode=1;
		}
		else {
			mode=0;
			oe.set_code_position(jmp1);
		}
	}

	bool op_flow_que::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		switch(mode) {
		case 1:
			oe.set_code_position(jmp2);
		case 0:
			oe.r=returnvalueflow::CONTINUE_OPCODES;
			break;
		case 2:
			RCASSERT(tmp.mode()==VAR_BOOL);
			update_opcodes(oe,tmp.get_bool());
			oe.r=returnvalueflow::CONTINUE_OPCODES;
			return true;
		}
		return false;
	}

	RCLMFUNCTION bool op_que_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_que *ff;
		unsigned int read;

		oe.push(ff,0);

		oe >> read >> ff->jmp1 >> ff->jmp2;

		RCASSERT(oe.tempstackactpos>=1);

		exec_variable &v=oe.temp(oe.tempstackactpos-1);

		if (v.mode()==VAR_BOOL) {
			ff->update_opcodes(oe,v.get_bool());
			--oe.tempstackactpos;
			return true;
		}
		bool b=oe.vm->calculate_bool(&ff->tmp,v);
		v.gc_release(*oe.vm);
		--oe.tempstackactpos;
		if (b) {
			ff->mode=2;
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
			return false;
		}

		ff->update_opcodes(oe,ff->tmp.get_bool());
		return true;
	}


} }












