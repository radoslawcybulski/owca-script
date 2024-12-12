#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_class.h"
#include "exec_class_object.h"
#include "virtualmachine.h"
#include "exec_stack.h"
#include "exec_object.h"
#include "exec_string.h"
#include "exec_property.h"
#include "exec_function_ptr.h"
#include "exec_function_stack_data.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_create_class_validate(opcode_validator &oe)
	{
		owca_internal_string *s;
		unsigned int inheritedcount;

		if (!oe.push_local_stack_data_size(sizeof(op_flow_class))) return false;
		if (!oe.get(inheritedcount)) return false; // amount of inherited classes
		if (oe.temporary_variables_count()<(unsigned int)inheritedcount+1) return false;
		if (!oe.get(s) || s->data_size() == 0) return false; // class name
		if (!oe.pop_temporary_variables(inheritedcount)) return false;
		//if (!oe.push_temporary_variable()) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_class))) return false;
		//oe.execution_can_continue=oe.continue_opcodes=true;
		return true;
	}

	RCLMFUNCTION bool op_create_class_flow(vm_execution_stack_elem_internal &oe)
	{
		unsigned int inheritedcount;

		oe >> inheritedcount;

		for(unsigned int i=0;i<inheritedcount;++i) {
			exec_variable &v=oe.temp(oe.tempstackactpos-inheritedcount+i);
			if (v.mode()!=VAR_OBJECT || !v.get_object()->is_type()) {
				oe.vm->raise_invalid_param(v,oe.vm->class_class);
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
				return false;
			}
		}

		op_flow_class *ff;
		oe.push(ff,0);
		ff->inheritedcount=inheritedcount;
		oe >> ff->name;

		exec_variable &fnc=oe.temp(oe.tempstackactpos-inheritedcount-1);
		RCASSERT(fnc.mode()==VAR_FUNCTION_FAST);
		RCASSERT(oe.vm->prepare_call_function(&fnc,fnc,NULL,0));
		ff->fnc=fnc.get_function_fast().fnc;
		fnc.reset();
		ff->stack=dynamic_cast<vm_execution_stack_elem_internal*>(oe.vm->execution_stack->peek_frame())->stack;
		ff->stack->gc_acquire();

		//vm_execution_stack_elem_internal *sf=oe.vm->push_execution_stack_frame_internal(stackdatasize,tempvarcount);

		//if (sf) {
		//	sf->return_value=&oe.temp(oe.tempstackactpos);
		//	sf->fnc=NULL;
		//	sf->opcodes=oe.opcodes;
		//	sf->opcodes_data=oe.opcodes_data;
		//	sf->opcodes_offset=oe.opcodes_offset;
		//	sf->r=returnvalueflow::CONTINUE_OPCODES;

		//	sf->stack=oe.vm->allocate_stack(oe.stack,scopesize);
		//	ff->stack=sf->stack;
		//	ff->stack->gc_acquire();
		//	sf->show_in_exception_stack(true);
		//}
		//else

		oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
		return false;
	}

	bool op_flow_class::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		exec_class_object *oo;
		exec_object *o=oe.vm->allocate_type(oo);
		oo->name=name;
		oo->name->gc_acquire();
		name=NULL;
		oo->constructable=true;
		oo->inheritable=true;

		// copy members based on stack identificators of created function into o' members
		for(unsigned int i=0;i<fnc->internal_variable_count();++i) {
			const exec_function_ptr::internal_variable_info &v=fnc->internal_variable(i);
			exec_variable &vv=stack->get_variable(v.location);
			o->set_member(*oe.vm,v.ident,vv);
		}

		std::string z=oo->_create(*oe.vm,&oe.temp(oe.tempstackactpos-inheritedcount),inheritedcount,o);
		if (z.empty()) {
			for(unsigned int i=inheritedcount;i>0;--i) {
				--oe.tempstackactpos;
				oe.temp(oe.tempstackactpos).gc_release(*oe.vm);
			}
			RCASSERT(oe.tempstackactpos>0);
			RCASSERT(oe.temp(oe.tempstackactpos-1).mode()==VAR_NULL);
			oe.temp(oe.tempstackactpos-1).set_object(o);
			oe.r=returnvalueflow::CONTINUE_OPCODES;
		}
		else {
			oe.vm->raise_class_creation(z);
			o->gc_release(*oe.vm);
			oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES;
		}
		return false;
	}

	void op_flow_class::_mark_gc(const gc_iteration &gc) const
	{
		{
			gc_iteration::debug_info _d("op_flow_class: stack object");
			stack->gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_class: fnc object");
			fnc->gc_mark(gc);
		}
	}

	void op_flow_class::_release_resources(virtual_machine &vm)
	{
		stack->gc_release(vm);
		fnc->gc_release(vm);
	}

} }












