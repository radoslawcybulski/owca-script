#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_lookup.h"
#include "virtualmachine.h"
#include "exec_string.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_lookup_read_validate(opcode_validator &oe)
	{
		owca_internal_string *s;

		if (oe.temporary_variables_count()==0) return false;
		if (!oe.get(s) || s->data_size()==0) return false;
		return true;
	}

	RCLMFUNCTION bool op_lookup_read_flow(vm_execution_stack_elem_internal &oe)
	{
		owca_internal_string *ident;
		exec_variable &v=oe.temp(oe.tempstackactpos-1),tmp;

		oe >> ident;
		tmp=v;
		lookupreturnvalue r=tmp.lookup_read(v,*oe.vm,ident);
		switch(r.type()) {
		case lookupreturnvalue::LOOKUP_FOUND:
			tmp.gc_release(*oe.vm);
			break;
		case lookupreturnvalue::LOOKUP_NOT_FOUND:
			oe.vm->raise_missing_member(tmp,ident->str());
		case lookupreturnvalue::LOOKUP_FUNCTION_CALL:
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
			tmp.gc_release(*oe.vm);
			v.reset();
			return false;
		}
		return true;
	}

	opcode_validator::boolean_result op_lookup_write_validate(opcode_validator &oe)
	{
		owca_internal_string *s;

		if (oe.temporary_variables_count()<2) return false;
		if (!oe.get(s) || s->data_size() == 0) return false;
		if (!oe.pop_temporary_variable()) return false;
		return true;
	}

	RCLMFUNCTION bool op_lookup_write_flow(vm_execution_stack_elem_internal &oe)
	{
		owca_internal_string *ident;
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);
		exec_variable value=params[1];
		params[1]=params[0];

		oe >> ident;
		lookupreturnvalue r=value.lookup_write(params[0],*oe.vm,ident,params[1]);
		switch(r.type()) {
		case lookupreturnvalue::LOOKUP_FOUND:
			value.gc_release(*oe.vm);
			params[1].gc_release(*oe.vm);
			--oe.tempstackactpos;
			break;
		case lookupreturnvalue::LOOKUP_NOT_FOUND:
			params[0].gc_release(*oe.vm);
			value.gc_release(*oe.vm);
			oe.tempstackactpos-=2;
			oe.vm->raise_missing_member(value,ident->str());
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
			return false;
		case lookupreturnvalue::LOOKUP_FUNCTION_CALL:
			params[0].reset();
			value.gc_release(*oe.vm);
			RCASSERT(oe.tempparamstrip==0);
			oe.tempparamstrip=1;
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
			return false;
		}
		return true;
	}

	opcode_validator::boolean_result op_lookup_write_oper_validate(opcode_validator &oe)
	{
		owca_internal_string *s;
		operatorcodes opcode;

		if (!oe.push_local_stack_data_size(sizeof(op_flow_lookup_oper))) return false;
		if (oe.temporary_variables_count()<2) return false;
		if (!oe.get(opcode)) return false;
		if (opcode!=E_ADD_SELF && opcode!=E_SUB_SELF && opcode!=E_MUL_SELF && opcode!=E_DIV_SELF && opcode!=E_MOD_SELF && opcode!=E_LSHIFT_SELF &&
			opcode!=E_RSHIFT_SELF && opcode!=E_BIN_AND_SELF && opcode!=E_BIN_OR_SELF && opcode!=E_BIN_XOR_SELF) return false;
		if (!oe.get(s) || s->data_size() == 0) return false;
		if (!oe.pop_temporary_variable()) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_lookup_oper))) return false;
		return true;
	}

	void op_flow_lookup_oper::_mark_gc(const gc_iteration &gc) const
	{
		{
			gc_iteration::debug_info _d("op_flow_lookup_oper: tmp");
			tmp.gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_lookup_oper: result");
			result.gc_mark(gc);
		}
	}

	void op_flow_lookup_oper::_release_resources(virtual_machine &vm)
	{
		tmp.gc_release(vm);
		result.gc_release(vm);
	}

	bool op_flow_lookup_oper::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		exec_variable *params=&oe.temp(oe.tempstackactpos-2);
		exec_variable &object=params[1];
		exec_variable &value=params[0];

		switch(mode) {
		CASE(0) {
			lookupreturnvalue r=object.lookup_read(tmp,*oe.vm,ident);
			switch(r.type()) {
			case lookupreturnvalue::LOOKUP_FOUND:
				break;
			case lookupreturnvalue::LOOKUP_NOT_FOUND:
				oe.vm->raise_missing_member(object,ident->str());
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
				return true;
			case lookupreturnvalue::LOOKUP_FUNCTION_CALL:
				mode=1;
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return true;
			default:
				RCASSERT(0);
			}
			GOTO(1); }
		CASE(1)
			if (!virtual_machine::calculate_nocall(oe.vm,result,opcode,tmp,value)) {
				exec_variable pp[2]={tmp,value};
				oe.vm->prepare_call_operator(&result,opcode,pp);
				mode=2;
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return true;
			}
			goto cont;
		CASE(2)
			if (!oe.vm->execution_self_oper) {
cont:
				tmp.gc_release(*oe.vm);
				lookupreturnvalue r=object.lookup_write(tmp,*oe.vm,ident,result);
				switch(r.type()) {
				case lookupreturnvalue::LOOKUP_FOUND:
					break;
				case lookupreturnvalue::LOOKUP_NOT_FOUND:
					tmp.reset();
					oe.vm->raise_missing_member(object,ident->str());
					oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
					return true;
				case lookupreturnvalue::LOOKUP_FUNCTION_CALL:
					tmp.reset();
					mode=3;
					oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
					return true;
				default:
					RCASSERT(0);
				}
			}
			else {
				object.gc_release(*oe.vm);
				value.gc_release(*oe.vm);
				--oe.tempstackactpos;
				params[0]=result;
				result.reset();
				oe.r=returnvalueflow::CONTINUE_OPCODES;
				return false;
			}
			GOTO(3);
		CASE(3)
			object.gc_release(*oe.vm);
			value.gc_release(*oe.vm);
			--oe.tempstackactpos;
			params[0]=tmp;
			result.reset();
			oe.r=returnvalueflow::CONTINUE_OPCODES;
			return false;
		default:
			RCASSERT(0);
		}
		return false;
	}

	RCLMFUNCTION bool op_lookup_write_oper_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=1);

		op_flow_lookup_oper *ff;
		oe.push(ff,0);

		ff->tmp.reset();
		ff->result.reset();

		oe >> ff->opcode >> ff->ident;
		ff->mode=0;

		oe.r=returnvalueflow::FIN;
		return false;
	}

} }

