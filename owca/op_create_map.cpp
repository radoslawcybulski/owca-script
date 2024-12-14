#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_create_map.h"
#include "returnvalue.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "exec_map_object.h"
#include "exec_tuple_object.h"
#include "exec_array_object.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_create_map_validate(opcode_validator &oe)
	{
		unsigned int cnt;
		if (!oe.get(cnt)) return false;
		if (cnt!=0) {
			if (!oe.push_local_stack_data_size(sizeof(op_flow_create_map))) return false;
			if (!oe.get_operands(cnt)) return false;
			if (!oe.pop_local_stack_data_size(sizeof(op_flow_create_map))) return false;
		}
		else {
			if (!oe.push_temporary_variable()) return false;
		}
		return true;
	}

	void op_flow_create_map::_mark_gc(const gc_iteration &gc) const
	{
		if (obj) {
			gc_iteration::debug_info _d("op_flow_create_map: obj object");
			obj->gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_create_map: tmp variable");
			tmp.gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_create_map: insert variable");
			insert.gc_mark(gc);
		}
	}

	void op_flow_create_map::_release_resources(virtual_machine &vm)
	{
		if (obj) obj->gc_release(vm);
		tmp.gc_release(vm);
		insert.gc_release(vm);
	}

	bool op_flow_create_map::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		switch(mode) {
		CASE(0) {
			exec_variable *params=&oe.temp(oe.tempstackactpos-count);
			tmp.gc_release(*oe.vm);
			tmp.reset();
			if (index<count) {
				RCASSERT(((count-index)&1)==0);

				oe.vm->prepare_call_function(&tmp,insert,params+index,2);
				index+=2;
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
			if (tmp.is_no_return_value()) { // done
				RCASSERT(generator==&oe.temp(oe.tempstackactpos-1));
				oe.r=returnvalueflow::CONTINUE_OPCODES;
				generator->gc_release(*oe.vm);
				generator->set_object(obj);
				obj=NULL;
				return false;
			}
			else {
				exec_tuple_object *to;
				exec_array_object *ao;
				exec_variable *v;

				if (tmp.mode()!=VAR_OBJECT) {
error:
					oe.vm->raise_invalid_param(OWCA_ERROR_FORMAT("generator didnt returned a tuple or list of size 2"));
					oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
					return false;
				}
				if ((to=oe.vm->data_from_object<exec_tuple_object>(tmp.get_object()))!=NULL) {
					if (to->size()!=2) goto error;
					v=&to->get(0);
				}
				else if ((ao=oe.vm->data_from_object<exec_array_object>(tmp.get_object()))!=NULL) {
					if (ao->size()!=2) goto error;
					v=&ao->get(0);
				}
				else goto error;

				oe.vm->prepare_call_function(&tmp,insert,v,2);
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

	RCLMFUNCTION bool op_create_map_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_map_object *oo;
		exec_object *o=oe.vm->allocate_map(oo);
		unsigned int count;

		oe >> count;

		if (count==0) {
			oe.temp(oe.tempstackactpos++).set_object(o);
			return true;
		}

		RCASSERT((count&1)==0);

		op_flow_create_map *ff;
		oe.push(ff,0);
		ff->count=count;
		ff->index=0;
		ff->obj=o;
		ff->oo=oo;
		ff->tmp.reset();
		ff->mode=0;

		exec_variable v;
		v.set_object(o);
		RCASSERT(v.lookup_operator(ff->insert,*oe.vm,E_ACCESS_1_WRITE));

		oe.r=returnvalueflow::FIN;
		return false;
	}

	opcode_validator::boolean_result op_create_map_comprehension_validate(opcode_validator &oe)
	{
		if (!oe.push_local_stack_data_size(sizeof(op_flow_create_map))) return false;
		if (!oe.get_operands(1)) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_create_map))) return false;
		return true;
	}

	RCLMFUNCTION bool op_create_map_comprehension_flow(vm_execution_stack_elem_internal &oe)
	{
		exec_map_object *oo;
		exec_object *o=oe.vm->allocate_map(oo);

		op_flow_create_map *ff;
		oe.push(ff,0);
		ff->obj=o;
		ff->oo=oo;
		ff->tmp.reset();
		ff->generator=&oe.temp(oe.tempstackactpos-1);
		ff->mode=1;

		exec_variable v;
		v.set_object(o);
		RCASSERT(v.lookup_operator(ff->insert,*oe.vm,E_ACCESS_1_WRITE));

		oe.r=returnvalueflow::FIN;
		return false;
	}

} }












