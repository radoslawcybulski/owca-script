#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_compare.h"
#include "exec_string.h"
#include "virtualmachine.h"
#include "exec_function_ptr.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_compare_simple_validate(opcode_validator &oe)
	{
		operatorcodes oper;
		if (!oe.get(oper)) return false;
		if (oper!=E_EQ && oper!=E_NOTEQ && oper!=E_LESS && oper!=E_MORE && oper!=E_LESSEQ && oper!=E_MOREEQ) return false;
		return oe.get_operands(2);
	}

	static bool compare(exec_variable &tmp, vm_execution_stack_elem_internal &oe, operatorcodes oper)
	{
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);
		exec_variable &retval=params[0];

		switch(oper) {
		case E_EQ:
			if (virtual_machine::calculate_eq_nocall(oe.vm,tmp,params[0],params[1])) return true;
			break;
		case E_NOTEQ:
			if (virtual_machine::calculate_noteq_nocall(oe.vm,tmp,params[0],params[1])) return true;
			break;
		case E_LESSEQ:
			if (virtual_machine::calculate_lesseq_nocall(oe.vm,tmp,params[0],params[1])) return true;
			break;
		case E_MOREEQ:
			if (virtual_machine::calculate_moreeq_nocall(oe.vm,tmp,params[0],params[1])) return true;
			break;
		case E_LESS:
			if (virtual_machine::calculate_less_nocall(oe.vm,tmp,params[0],params[1])) return true;
			break;
		case E_MORE:
			if (virtual_machine::calculate_more_nocall(oe.vm,tmp,params[0],params[1])) return true;
			break;
		default:
			RCASSERT(0);
		}

		return false;
	}

	RCLMFUNCTION bool op_compare_simple_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=2);
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);
		exec_variable &retval=params[0],tmp;
		operatorcodes oper;

		oe >> oper;
		if (compare(tmp,oe,oper)) {
			params[0].gc_release(*oe.vm);
			params[1].gc_release(*oe.vm);
			params[0]=tmp;
			--oe.tempstackactpos;
			return true;
		}

		oe.prepare_call_operator_stack(oper);
		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		return false;
	}

	opcode_validator::boolean_result op_compare_validate(opcode_validator &oe)
	{
		unsigned int cnt;
		opcode_executer_jump jmp;

		if (!oe.get(cnt) || cnt<2 || !oe.get(jmp)) return false;
		if (!oe.push_local_stack_data_size(sizeof(op_flow_compare))) return false;
		for(unsigned int i=0;i<cnt;++i) {
			if (!oe.validate_read_expr()) return false;
			if (!oe.push_temporary_variable()) return false;
			operatorcodes oper;
			if (!oe.get(oper)) return false;
			if (oper!=E_EQ && oper!=E_NOTEQ && oper!=E_LESS && oper!=E_MORE && oper!=E_LESSEQ && oper!=E_MOREEQ) return false;
		}
		if (oe.compare(jmp)!=0) return false;
		if (!oe.get_operands(cnt+1)) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_compare))) return false;
		oe.execution_can_continue=oe.continue_opcodes=true;
		return true;
	}

	bool op_flow_compare::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		returnvalue r;
		operatorcodes oper;
		exec_variable *params = NULL;

		switch(mode) {
		case 0: {
			oe >> oper;
			RCASSERT(oe.tempstackactpos>=2);
			params=&oe.temp(oe.tempstackactpos-2);
			if (compare(tmp,oe,oper)) {
parse:
				if (!tmp.get_bool() || index>=count-1) {
					params=&oe.temp(oe.tempstackactpos-(index+2));
					for(unsigned int i=index+2;i>0;--i) {
						params[i-1].gc_release(*oe.vm);
					}
					oe.tempstackactpos-=index+1;
					params[0]=tmp;
					oe.set_code_position(done);
					oe.r=returnvalueflow::CONTINUE_OPCODES;
					return false;
				}
				++index;
				oe.r=returnvalueflow::CONTINUE_OPCODES;
				return true;
			}
			oe.vm->prepare_call_operator(&tmp,oper,params);
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
			mode=1;
			return true; }
		case 1:
			mode=0;
			goto parse;
		default:
			RCASSERT(0);
			return false;
		}
	}

	RCLMFUNCTION bool op_compare_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_compare *ff;

		oe.push(ff,0);
		ff->index=0;
		oe >> ff->count;
		oe >> ff->done;
		ff->mode=0;
		return true;
	}

} }


