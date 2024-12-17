#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_with.h"
#include "virtualmachine.h"
#include "exec_stack.h"
#include "exec_base.h"
#include "exec_string.h"
#include "exec_object.h"
#include "returnvalue.h"
#include "vm_execution_stack_elem_internal.h"

namespace owca { namespace __owca__ {

	extern std::string identificator_names[];

	unsigned int op_flow_with::size() const
	{
		return sizeof(*this)+sizeof(exec_variable)*cnt;
	}

	bool op_flow_with::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		switch(mode) {
		case 0: {
			RCASSERT(oe.tempstackactpos==1);
			exec_variable &v=oe.temp(0);
			variables()[act]=v;
			//if (v.has_operator(*oe.vm,E_WITH_ENTER)) {
			mode=1;
			if (v.mode()!=VAR_FUNCTION && v.mode()!=VAR_FUNCTION_FAST) {
				oe.vm->prepare_call_operator(&v,E_WITH_ENTER,&v);
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				v.reset();
				return true;
			}
			else {
				v.gc_acquire();
			}
			//}
			//v.gc_acquire();
			}
		case 1: {
			RCASSERT(oe.tempstackactpos==1);
			exec_variable &v=oe.temp(0);
			oe.tempstackactpos=0;
			exec_variable_location vl;
			oe >> vl;
			if (vl.valid()) {
				exec_variable &vv=oe.get(vl);
				vv.gc_release(*oe.vm);
				vv=v;
			}
			else v.gc_release(*oe.vm);
			++act;
			if (act == cnt) {
				mode = 10;
			}
			else mode=0;
			back=oe.r=returnvalueflow::CONTINUE_OPCODES;
			return true; }
		case 10:
			mode=11;
			excobj=oe.vm->execution_exception_object_thrown;
			resume=oe.actual_code_position();
			oe.set_code_position(start);
		case 11:
			if (!callvars_prepare(*oe.vm)) {
				oe.r=excobj!=oe.vm->execution_exception_object_thrown ? returnvalueflow::EXCEPTION : back;
				oe.set_code_position(resume);
				return false;
			}
			else {
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return true;
			}
		default:
			RCASSERT(0);
		}
		return false;
	}

	bool op_flow_with::resume_return(vm_execution_stack_elem_internal &oe)
	{
		back=oe.r;
		RCASSERT(mode==10);
		return op_flow_with::resume_fin(oe);
	}

	bool op_flow_with::resume_loop_control(vm_execution_stack_elem_internal &oe)
	{
		back=oe.r;
		RCASSERT(mode==10);
		return op_flow_with::resume_fin(oe);
	}

	bool op_flow_with::resume_exception(vm_execution_stack_elem_internal &oe)
	{
		back=oe.r;
		mode=10;
		return op_flow_with::resume_fin(oe);
	}

	bool op_flow_with::callvars_prepare(virtual_machine &vm)
	{
		tmp.gc_release(vm);
		tmp.reset();

		// with statement is being pop'ed due to an exception in one of variables it uses
		// so variables[act] failed to call $withenter, but it contains valid reference
		// and this reference need to be dereferenced manually
		if (act<cnt) {
			exec_variable &v=variables()[act];
			v.gc_release(vm);
			v.setmode(VAR_NO_DEF_VALUE);
		}
		for(;;) {
			if (act==0) return false;
			--act;
			exec_variable &v=variables()[act];
			switch(v.mode()) {
			case VAR_NO_DEF_VALUE:
				continue;
			case VAR_FUNCTION:
			case VAR_FUNCTION_FAST:
				vm.prepare_call_function(&tmp,v,NULL,0);
				break;
			default:
				vm.prepare_call_operator(&tmp,E_WITH_EXIT,&v);
				break;
			}
			v.gc_release(vm);
			v.setmode(VAR_NO_DEF_VALUE);
			return true;
		}
	}

	void op_flow_with::_mark_gc(const gc_iteration &gc) const
	{
		for(unsigned int i=0;i<cnt;++i) {
			gc_iteration::debug_info _d("op_flow_with: locked variable %d",i);
			variables()[i].gc_mark(gc);
		}
		{
			gc_iteration::debug_info _d("op_flow_with: tmp variable");
			tmp.gc_mark(gc);
		}
	}

	void op_flow_with::_release_resources(virtual_machine &vm) // this will call exit functions even in case of generator object being deleted without completing first
	{
		auto pop = vm.push_execution_stack();

		while(callvars_prepare(vm)) {
			try {
				vm.execute_stack();
			}
			catch (...) {
				tmp.gc_release(vm);
				tmp.reset();
			}
		}
		tmp.gc_release(vm);
		tmp.reset();
	}

	opcode_validator::boolean_result op_with_validate(opcode_validator &oe)
	{
		unsigned int cnt;
		if (!oe.get(cnt) || cnt==0) return false;
		exec_variable_location loc;

		if (!oe.push_local_stack_data_size(sizeof(op_flow_with)+sizeof(exec_variable)*cnt)) return false;
		for(unsigned int i=0;i<cnt;++i) {
			if (!oe.validate_read_expr()) return false;
			if (!oe.get(loc) || (loc.valid() && !oe.check(loc))) return false;
		}
		if (!oe.validate_flow()) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_with)+sizeof(exec_variable)*cnt)) return false;
		oe.continue_opcodes=true;
		return true;
	}

	//static owca_internal_string_nongc *s=owca_internal_string_nongc::allocate("close");

	RCLMFUNCTION bool op_with_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_with *ff;
		RCASSERT(oe.tempstackactpos==0);

		unsigned int cnt;
		oe >> cnt;

		oe.push(ff,cnt*sizeof(exec_variable));
		ff->start=oe.actual_code_position();
		ff->cnt=cnt;
		ff->act=0;
		ff->mode=0;
		ff->tmp.reset();
		for(unsigned int i=0;i<cnt;++i) ff->variables()[i].setmode(VAR_NO_DEF_VALUE);

		return true;
	}

} }












