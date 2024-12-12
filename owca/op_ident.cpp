#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_ident.h"
#include "exec_stack.h"
#include "virtualmachine.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"
#include "exec_property.h"
#include "exec_function_ptr.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_ident_read_validate(opcode_validator &oe)
	{
		exec_variable_location vl;
		if (!oe.get(vl) || !oe.check(vl)) return false;
		if (!oe.push_temporary_variable()) return false;
		return true;
	}

	RCLMFUNCTION bool op_ident_read_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_variable &retval=oe.temp(oe.tempstackactpos++);

		exec_variable_location vl;
		oe >> vl;
		retval=vl.depth()==0 ? oe.get0(vl.offset()) : oe.get_local(vl);
		retval.gc_acquire();
		return true;
	}

	opcode_validator::boolean_result op_ident_write_validate(opcode_validator &oe)
	{
		if (!oe.get_operands(1)) return false;
		exec_variable_location vl;
		if (!oe.get(vl) || !oe.check(vl)) return false;
		return true;
	}

	RCLMFUNCTION bool op_ident_write_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=1);
		exec_variable *params=&oe.temp(oe.tempstackactpos-1);

		exec_variable_location vl;
		oe >> vl;
		exec_variable &v=vl.depth()==0 ? oe.get0(vl.offset()) : oe.get_local(vl);
		v.gc_release(*oe.vm);
		v=params[0];
		v.gc_acquire();
		return true;
	}

	opcode_validator::boolean_result op_ident_write_property_validate(opcode_validator &oe)
	{
		if (!oe.get_operands(1)) return false;
		exec_variable_location vl;
		if (!oe.get(vl) || !oe.check(vl)) return false;
		return true;
	}

	RCLMFUNCTION bool op_ident_write_property_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=1);
		exec_variable *params=&oe.temp(oe.tempstackactpos-1);

		RCASSERT(params[0].mode()==VAR_FUNCTION_FAST);
		RCASSERT(params[0].get_function_fast().slf==NULL);

		exec_function_ptr *fncptr=params[0].get_function_fast().fnc;

		exec_variable_location vl;
		oe >> vl;
		exec_variable &tmp=vl.depth()==0 ? oe.get0(vl.offset()) : oe.get_local(vl);

		exec_property *p;
		if (tmp.mode()!=VAR_PROPERTY) {
			p=oe.vm->allocate_property();

			switch(fncptr->internal_function_type()) {
			case exec_function_ptr::PROPERTY_READ:
				p->read=fncptr;
				break;
			case exec_function_ptr::PROPERTY_WRITE:
				p->write=fncptr;
				break;
			default:
				RCASSERT(0);
			}
			tmp.gc_release(*oe.vm);
			tmp.set_property(p);
		}
		else {
			p=tmp.get_property();

			switch(fncptr->internal_function_type()) {
			case exec_function_ptr::PROPERTY_READ:
				if (p->read) p->read->gc_release(*oe.vm);
				p->read=fncptr;
				break;
			case exec_function_ptr::PROPERTY_WRITE:
				if (p->write) p->write->gc_release(*oe.vm);
				p->write=fncptr;
				break;
			default:
				RCASSERT(0);
			}
		}

		params[0].set_null();

		return true;
	}

	opcode_validator::boolean_result op_ident_write_oper_validate(opcode_validator &oe)
	{
		if (!oe.push_local_stack_data_size(sizeof(op_flow_ident_oper))) return false;
		if (!oe.get_operands(1)) return false;
		exec_variable_location vl;
		operatorcodes opcode;
		if (!oe.get(vl) || !oe.check(vl)) return false;
		if (!oe.get(opcode)) return false;
		if (opcode!=E_ADD_SELF && opcode!=E_SUB_SELF && opcode!=E_MUL_SELF && opcode!=E_DIV_SELF && opcode!=E_MOD_SELF && opcode!=E_LSHIFT_SELF &&
			opcode!=E_RSHIFT_SELF && opcode!=E_BIN_AND_SELF && opcode!=E_BIN_OR_SELF && opcode!=E_BIN_XOR_SELF) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_ident_oper))) return false;
		return true;
	}

	void op_flow_ident_oper::_mark_gc(const gc_iteration &gc) const
	{
	}

	void op_flow_ident_oper::_release_resources(virtual_machine &vm)
	{
	}

	bool op_flow_ident_oper::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		oe.r=returnvalueflow::CONTINUE_OPCODES;
		if (!oe.vm->execution_self_oper) {
			dst->gc_release(*oe.vm);
			*dst=*src;
			dst->gc_acquire();
		}
		return false;
	}

	RCLMFUNCTION bool op_ident_write_oper_flow(vm_execution_stack_elem_internal &oe)
	{
		RCASSERT(oe.tempstackactpos>=1);
		exec_variable *params=&oe.temp(oe.tempstackactpos-1);

		exec_variable_location vl;
		operatorcodes opcode;
		oe >> vl >> opcode;

		exec_variable &v=vl.depth()==0 ? oe.get0(vl.offset()) : oe.get_local(vl);

		exec_variable tmp;
		op_flow_ident_oper *ff;

		if (virtual_machine::calculate_nocall(oe.vm,tmp,opcode,v,params[0])) {
			params[0].gc_release(*oe.vm);
			params[0]=tmp;
			v.gc_release(*oe.vm);
			v=tmp;
			v.gc_acquire();
			return true;
		}

		oe.push(ff,0); // E_xxx_SELF shouldnt write to v, only to temporaries
		exec_variable fparams[2]={v,params[0]};

		ff->dst=&v;
		ff->src=params;

		oe.vm->prepare_call_operator(params,opcode,fparams);
		params[0].gc_release(*oe.vm);
		params[0].reset();

		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
		return false;
	}

} }


