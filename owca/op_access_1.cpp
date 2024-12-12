#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_access_1.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "exec_string.h"
#include "vm_execution_stack_elem_internal.h"
#include "exec_namespace.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_access_1_read_validate(opcode_validator &oe)
	{
		return oe.get_operands(2);
	}

	bool string_read_1(exec_variable &dst, owca_internal_string *s, virtual_machine &vm, owca_int i1);

	RCLMFUNCTION bool op_access_1_read_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=2);
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);
		exec_variable &retval=params[0];

		switch(params[0].mode()) {
		case VAR_NULL:
		case VAR_INT:
		case VAR_REAL:
		case VAR_BOOL:
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_OBJECT:
		case VAR_WEAK_REF:
		case VAR_COUNT:
		case VAR_HASH_DELETED:
		case VAR_HASH_EMPTY:
		case VAR_NO_DEF_VALUE:
		case VAR_NO_PARAM_GIVEN:
		case VAR_UNDEFINED:
			break;
		case VAR_STRING: {
			owca_int i;
			if (params[1].get_int(i)) {
				exec_variable v;
				if (string_read_1(v,params[0].get_string(),*oe.vm,i)) {
					params[0].gc_release(*oe.vm);
					params[0]=v;
					oe.tempstackactpos-=(2-1);
					return true;
				}
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
				return false;
			}
			break; }
		case VAR_NAMESPACE:
			if (params[1].mode()==VAR_STRING) {
				exec_variable v;
				if (params[0].get_namespace()->get_variable(params[1].get_string(),v)) {
					params[0].gc_release(*oe.vm);
					params[1].gc_release(*oe.vm);
					params[0]=v;
					oe.tempstackactpos-=(2-1);
					return true;
				}
				oe.vm->raise_missing_member(params[0],params[1].get_string()->str());
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
				return false;
			}
			break;
		}
		oe.prepare_call_operator_stack(E_ACCESS_1_READ);
		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		return false;
	}

	opcode_validator::boolean_result op_access_1_write_validate(opcode_validator &oe)
	{
		return oe.get_operands(3);
	}

	RCLMFUNCTION bool op_access_1_write_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=3);
		exec_variable *params=&oe.temp(oe.tempstackactpos-3);
		exec_variable &retval=params[0];

		switch(params[0].mode()) {
		case VAR_NULL:
		case VAR_INT:
		case VAR_REAL:
		case VAR_BOOL:
		case VAR_GENERATOR:
		case VAR_PROPERTY:
		case VAR_FUNCTION:
		case VAR_FUNCTION_FAST:
		case VAR_OBJECT:
		case VAR_WEAK_REF:
		case VAR_COUNT:
		case VAR_HASH_DELETED:
		case VAR_HASH_EMPTY:
		case VAR_NO_DEF_VALUE:
		case VAR_NO_PARAM_GIVEN:
		case VAR_UNDEFINED:
		case VAR_STRING:
			break;
		case VAR_NAMESPACE:
			if (params[1].mode()==VAR_STRING) {
				RCASSERT(params[0].get_namespace()->insert_variable(params[1].get_string(),params[2])>=0);
				params[0].gc_release(*oe.vm);
				params[1].gc_release(*oe.vm);
				params[0]=params[2];
				oe.tempstackactpos-=(3-1);
				return true;

				//oe.vm->raise_not_lvalue_member(params[0],params[1].get_string()->str());
				//oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
				//return false;
			}
			break;
		}
		oe.prepare_call_operator_stack(E_ACCESS_1_WRITE);
		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		return false;
	}

	opcode_validator::boolean_result op_access_1_write_oper_validate(opcode_validator &oe)
	{
		operatorcodes opcode;

		if (!oe.get_operands(3)) return false;
		if (!oe.push_local_stack_data_size(sizeof(op_flow_access_1_oper))) return false;
		if (!oe.get(opcode)) return false;
		if (opcode!=E_ADD_SELF && opcode!=E_SUB_SELF && opcode!=E_MUL_SELF && opcode!=E_DIV_SELF && opcode!=E_MOD_SELF && opcode!=E_LSHIFT_SELF &&
			opcode!=E_RSHIFT_SELF && opcode!=E_BIN_AND_SELF && opcode!=E_BIN_OR_SELF && opcode!=E_BIN_XOR_SELF) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_access_1_oper))) return false;
		return true;
	}

	void op_flow_access_1_oper::_mark_gc(const gc_iteration &gc) const
	{
		{
			gc_iteration::debug_info _d("op_flow_access_1_oper: tmp variable");
			tmp.gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_access_1_oper: result variable");
			result.gc_mark(gc);
		}
	}

	void op_flow_access_1_oper::_release_resources(virtual_machine &vm)
	{
		tmp.gc_release(vm);
		result.gc_release(vm);
	}

	bool op_flow_access_1_oper::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		exec_variable *params=&oe.temp(oe.tempstackactpos-3);
		exec_variable &index_1=params[2];
		exec_variable &object=params[1];
		exec_variable &value=params[0];

		switch(mode) {
		CASE(0)
			oe.vm->prepare_call_operator(&tmp,E_ACCESS_1_READ,&object);
			mode=1;
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
			return true;
		CASE(1)
			if (!virtual_machine::calculate_nocall(oe.vm,result,opcode,tmp,value)) {
				exec_variable pp[]={tmp,value};
				oe.vm->prepare_call_operator(&result,opcode,pp);
				mode=2;
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return true;
			}
			goto cont;
		CASE(2)
			if (!oe.vm->execution_self_oper) {
cont:
				exec_variable pp[]={object,index_1,result};
				oe.vm->prepare_call_operator(&result,E_ACCESS_1_WRITE,pp);
				mode=3;
				result.gc_release(*oe.vm);
				result.reset();
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return true;
			}
			else {
				object.gc_release(*oe.vm);
				value.gc_release(*oe.vm);
				index_1.gc_release(*oe.vm);
				oe.tempstackactpos-=2;
				params[0]=result;
				result.reset();
				oe.r=returnvalueflow::CONTINUE_OPCODES;
				return false;
			}
			GOTO(3);
		CASE(3)
			object.gc_release(*oe.vm);
			value.gc_release(*oe.vm);
			index_1.gc_release(*oe.vm);
			oe.tempstackactpos-=2;
			params[0]=result;
			result.reset();
			oe.r=returnvalueflow::CONTINUE_OPCODES;
			return false;
		default:
			RCASSERT(0);
		}
		return false;
	}

	RCLMFUNCTION bool op_access_1_write_oper_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=3);

		op_flow_access_1_oper *ff;
		oe.push(ff,0);

		ff->tmp.reset();
		ff->result.reset();

		oe >> ff->opcode;
		ff->mode=0;

		oe.r=returnvalueflow::FIN;
		return false;
	}

} }


