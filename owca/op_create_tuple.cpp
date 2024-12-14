#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_create_tuple.h"
#include "returnvalue.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "exec_tuple_object.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_create_tuple_validate(opcode_validator &oe)
	{
		unsigned int cnt;
		if (!oe.get(cnt)) return false;
		if (cnt==0) {
			if (!oe.push_temporary_variable()) return false;
			return true;
		}
		return oe.get_operands(cnt);
	}

	RCLMFUNCTION bool op_create_tuple_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_object *o;
		exec_tuple_object *oo;
		unsigned int count;
		oe >> count;
		o=oe.vm->allocate_tuple(oo,count);

		for(unsigned int i=0;i<count;++i) {
			oo->get(i)=oe.temp(oe.tempstackactpos-count+i);
		}

		oe.temp(oe.tempstackactpos-count).set_object(o);
		oe.tempstackactpos-=(count-1);

		return true;
	}


	void op_flow_create_tuple::_mark_gc(const gc_iteration &gc) const
	{
		unsigned int index=0;
		for(std::list<exec_variable>::const_iterator it=vars.begin();it!=vars.end();++it,++index) {
			gc_iteration::debug_info _d("op_flow_create_tuple: tmp variable %d",index);
			it->gc_mark(gc);
		}
	}

	void op_flow_create_tuple::_release_resources(virtual_machine &vm)
	{
		for(std::list<exec_variable>::iterator it=vars.begin();it!=vars.end();++it) {
			it->gc_release(vm);
		}
	}

	bool op_flow_create_tuple::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		switch(mode) {
		CASE(0)
			vars.push_back(exec_variable());
			vars.back().reset();
			oe.vm->prepare_call_function(&vars.back(),*generator,NULL,0);
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
			mode=1;
			return true;
		CASE(1)
			if (vars.back().is_no_return_value()) { // done
				RCASSERT(generator==&oe.temp(oe.tempstackactpos-1));
				oe.r=returnvalueflow::CONTINUE_OPCODES;
				generator->gc_release(*oe.vm);

				vars.pop_back();

				exec_tuple_object *oo;
				generator->set_object(oe.vm->allocate_tuple(oo, (unsigned int)vars.size()));
				unsigned int index=0;
				while(!vars.empty()) {
					oo->get(index++)=vars.front();
					vars.pop_front();
				}

				return false;
			}
			GOTO(0);
		default:
			RCASSERT(0);
		}
		return false;
	}

	opcode_validator::boolean_result op_create_tuple_comprehension_validate(opcode_validator &oe)
	{
		if (!oe.push_local_stack_data_size(sizeof(op_flow_create_tuple))) return false;
		if (!oe.get_operands(1)) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_create_tuple))) return false;
		return true;
	}

	RCLMFUNCTION bool op_create_tuple_comprehension_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_create_tuple *ff;
		oe.push(ff,0);
		ff->generator=&oe.temp(oe.tempstackactpos-1);
		ff->mode=0;

		oe.r=returnvalueflow::FIN;
		return false;
	}

} }












