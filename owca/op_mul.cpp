#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "virtualmachine.h"
#include "exec_string.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	owca_internal_string *string_mul(virtual_machine *vm, owca_internal_string *s, unsigned int i);

	opcode_validator::boolean_result op_mul_validate(opcode_validator &oe)
	{
		return oe.get_operands(2);
	}

	RCLMFUNCTION bool op_mul_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=2);
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);
		exec_variable &retval=params[0];

		owca_internal_string *ss;
		owca_int i;

		switch(params[1].mode()==VAR_OBJECT ? VAR_OBJECT : params[0].mode()) {
		case VAR_INT:
			i=params[0].get_int();
			switch(params[1].mode()) {
			case VAR_INT:
				retval.set_int(i*params[1].get_int());
				break;
			case VAR_REAL:
				retval.set_real(i*params[1].get_real());
				break;
			case VAR_STRING:
				ss=params[1].get_string();
process:
				if (i<0) {
neg:
					oe.vm->raise_invalid_integer(OWCA_ERROR_FORMAT("cant multiply by negative amount"));
					oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
					return false;
				}
				else {
					owca_internal_string *s=string_mul(oe.vm,ss,(unsigned int)i);
					params[1].gc_release(*oe.vm);
					retval.set_string(s);
				}
				break;
			default:
				goto cont;
			}
			break;
		case VAR_REAL:
			switch(params[1].mode()) {
			case VAR_INT:
				retval.set_real(params[0].get_real()*params[1].get_int());
				break;
			case VAR_REAL:
				retval.set_real(params[0].get_real()*params[1].get_real());
				break;
			case VAR_STRING:
				i=(owca_int)params[0].get_real();
				if (i==params[0].get_real()) {
					ss=params[1].get_string();
					goto process;
				}
				else goto cont;
				break;
			default:
				goto cont;
			}
			break;
		case VAR_STRING:
			if (params[1].get_int(i)) {
				if (i<0) goto neg;
				owca_internal_string *s=string_mul(oe.vm,params[0].get_string(),(unsigned int)i);
				params[0].gc_release(*oe.vm);
				retval.set_string(s);
			}
			else goto cont;
			break;
		default:
cont:
			oe.prepare_call_operator_stack(E_MUL);
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
			return false;
		}
		--oe.tempstackactpos;
		return true;
	}

} }


