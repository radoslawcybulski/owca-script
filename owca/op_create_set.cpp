#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_create_set.h"
#include "returnvalue.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "exec_set_object.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_create_set_validate(opcode_validator &oe)
	{
		unsigned int cnt;
		if (!oe.get(cnt)) return false;
		if (cnt!=0) {
			if (!oe.push_local_stack_data_size(sizeof(op_flow_create_set))) return false;
			if (!oe.get_operands(cnt)) return false;
			if (!oe.pop_local_stack_data_size(sizeof(op_flow_create_set))) return false;
		}
		else {
			if (!oe.push_temporary_variable()) return false;
		}
		return true;
	}

	void op_flow_create_set::_mark_gc(const gc_iteration &gc) const
	{
		if (obj) {
			gc_iteration::debug_info _d("op_flow_create_set: obj object");
			obj->gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_create_set: tmp variable");
			tmp.gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_create_set: insert variable");
			insert.gc_mark(gc);
		}
	}

	void op_flow_create_set::_release_resources(virtual_machine &vm)
	{
		if (obj) obj->gc_release(vm);
		tmp.gc_release(vm);
		insert.gc_release(vm);
	}

	bool op_flow_create_set::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		switch(mode) {
		CASE(0) {
			exec_variable *params=&oe.temp(oe.tempstackactpos-count);
			tmp.gc_release(*oe.vm);
			tmp.reset();
			if (index<count) {
				oe.vm->prepare_call_function(&tmp,insert,params+index,1);
				++index;
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return true;
			}
			else {
				oe.r=returnvalueflow::CONTINUE_OPCODES;
				for(unsigned int i=count;i>0;--i) {
					params[i-1].gc_release(*oe.vm);
				}
				params[0].set_object(obj);
				obj=NULL;
				oe.tempstackactpos-=(count-1);
				return false;
			} }
		CASE(1)
			tmp.gc_release(*oe.vm);
			tmp.reset();
			oe.vm->prepare_call_function(&tmp,*generator,NULL,0);
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
			mode=2;
			return true;
		CASE(2)
			if (oe.vm->execution_no_return_value) { // done
				RCASSERT(generator==&oe.temp(oe.tempstackactpos-1));
				oe.r=returnvalueflow::CONTINUE_OPCODES;
				generator->gc_release(*oe.vm);
				generator->set_object(obj);
				obj=NULL;
				return false;
			}
			else {
				oe.vm->prepare_call_function(&tmp,insert,&tmp,1);
				tmp.gc_release(*oe.vm);
				tmp.reset();
				mode=1;
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return true;
			}
		default:
			RCASSERT(0);
		}
		return false;
	}

	RCLMFUNCTION bool op_create_set_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_set_object *oo;
		exec_object *o=oe.vm->allocate_set(oo);
		unsigned int count;

		oe >> count;

		if (count==0) {
			oe.temp(oe.tempstackactpos++).set_object(o);
			return true;
		}

		op_flow_create_set *ff;
		oe.push(ff,0);
		ff->count=count;
		ff->index=0;
		ff->obj=o;
		ff->oo=oo;
		ff->tmp.reset();
		ff->mode=0;

		exec_variable v;
		v.set_object(o);
		RCASSERT(v.lookup_operator(ff->insert,*oe.vm,E_BIN_OR_SELF));

		oe.r=returnvalueflow::FIN;
		return false;
	}

	opcode_validator::boolean_result op_create_set_comprehension_validate(opcode_validator &oe)
	{
		if (!oe.push_local_stack_data_size(sizeof(op_flow_create_set))) return false;
		if (!oe.get_operands(1)) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_create_set))) return false;
		return true;
	}

	RCLMFUNCTION bool op_create_set_comprehension_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_set_object *oo;
		exec_object *o=oe.vm->allocate_set(oo);

		op_flow_create_set *ff;
		oe.push(ff,0);
		ff->obj=o;
		ff->oo=oo;
		ff->tmp.reset();
		ff->generator=&oe.temp(oe.tempstackactpos-1);
		ff->mode=1;

		exec_variable v;
		v.set_object(o);
		RCASSERT(v.lookup_operator(ff->insert,*oe.vm,E_BIN_OR_SELF));

		oe.r=returnvalueflow::FIN;
		return false;
	}

} }












