#include "stdafx.h"
#include "base.h"
#include "op_base.h"
#include "op_write.h"
#include "op_validate.h"
#include "op_try.h"
#include "virtualmachine.h"
#include "exec_stack.h"
#include "exec_base.h"
#include "exec_object.h"
#include "returnvalue.h"
#include "exec_variablelocation.h"
#include "vm_execution_stack_elem_internal.h"

#pragma warning(disable : 4533)
namespace owca { namespace __owca__ {

	opcode_validator::boolean_result op_try_validate(opcode_validator &oe)
	{
		opcode_executer_jump blocks,elseblock,finallyblock,done;

		if (oe.temporary_variables_count()!=0) return false;
		if (!oe.push_local_stack_data_size(sizeof(op_flow_try))) return false;

		if (!oe.get(blocks) || !oe.get(elseblock) || !oe.get(finallyblock) || !oe.get(done)) return false;
		if (!oe.validate_flow()) return false;
		if (oe.compare(blocks)!=0) return false;

		unsigned int cnt;
		if (!oe.get(cnt) || cnt==0) return false;
		for(unsigned int i=0;i<cnt;++i) {
			opcode_executer_jump jmp1,jmp2;
			unsigned int cnt2;

			if (!oe.get(jmp1) || !oe.get(jmp2) || !oe.get(cnt2)) return false;
			for(unsigned int j=0;j<cnt2;++j) {
				if (!oe.validate_read_expr()) return false;
			}
			if (oe.compare(jmp1)!=0) return false;
			exec_variable_location vl;
			if (!oe.get(vl)) return false;
			if (vl.valid() && !oe.check(vl)) return false;
			if (!oe.validate_flow()) return false;
			if (oe.compare(jmp2)!=0) return false;
		}
		if (elseblock != done) {
			if (oe.compare(elseblock)!=0) return false;
			if (!oe.validate_flow()) return false;
		}
		if (finallyblock != done) {
			if (oe.compare(finallyblock)!=0) return false;
			if (!oe.validate_flow()) return false;
		}

		if (oe.compare(done)!=0) return false;
		if (!oe.pop_local_stack_data_size(sizeof(op_flow_try))) return false;
		oe.execution_can_continue=oe.continue_opcodes=true;
		return true;
	}

	void op_flow_try::_mark_gc(const gc_iteration &gc) const
	{
		if (old_being_handled) {
			gc_iteration::debug_info _d("op_flow_try: previous exception object");
			old_being_handled->gc_mark(gc);
		}
	}

	void op_flow_try::_release_resources(virtual_machine &vm)
	{
		if (old_being_handled) old_being_handled->gc_release(vm);
	}

	bool op_flow_try::set_dest_else_finally(vm_execution_stack_elem_internal &oe, const vm_execution_stack_elem_internal_jump &j)
	{
		if (j == done) {
			oe.set_code_position(done);
			return false;
		}
		oe.set_code_position(j);
		mode=1;
		return true;
	}

	RCLMFUNCTION bool op_flow_try::resume_fin(vm_execution_stack_elem_internal &oe)
	{
		oe.r=returnvalueflow::CONTINUE_OPCODES;
		switch(mode) {
		case 0:
			mode=1;
			return set_dest_else_finally(oe,elseblock);
		case 1:
			oe.set_code_position(done);
			return false;
		case 9:
			oe.set_code_position(blocks);
			oe >> blockcount;
			RCASSERT(blockcount>0);
		case 10:
next_block:
			oe >> jmp1 >> jmp2 >> blocksubcount;
			if (blocksubcount==0) {
				// default handler
matched:
				oe.set_code_position(jmp1);
				exec_variable_location vl;
				oe >> vl;
				if (vl.valid()) {
					exec_variable &v=oe.get(vl);
					v.gc_release(*oe.vm);
					v.set_object(oe.vm->execution_exception_object_thrown);
					oe.vm->execution_exception_object_thrown->gc_acquire();
				}
				old_being_handled=oe.exception_object_being_handled;
				oe.exception_object_being_handled=oe.vm->execution_exception_object_thrown;
				oe.vm->execution_exception_object_thrown=NULL;
				mode=30;
			}
			else {
				mode=11;
			}
			return true;
		case 11: {
			RCASSERT(oe.tempstackactpos==1);
			exec_variable &v=oe.temp(0);
			if (v.mode()!=VAR_OBJECT || !v.get_object()->is_type() || !v.get_object()->inherit_from(oe.vm->class_exception)) {
				RCASSERT(oe.exception_object_being_handled==NULL);
				oe.exception_object_being_handled=oe.vm->execution_exception_object_thrown;
				oe.vm->execution_exception_object_thrown=NULL;
				oe.vm->raise_invalid_param(v,oe.vm->class_exception);
				oe.r=returnvalueflow::CALL_FUNCTION_CONTINUE_FIN;
				return false;
			}
			bool match=v.mode()==VAR_OBJECT && v.get_object()->is_type() && oe.vm->execution_exception_object_thrown->type()->inherit_from(v.get_object());
			v.gc_release(*oe.vm);
			oe.tempstackactpos=0;
			if (match) goto matched;
			--blocksubcount;
			if (blocksubcount>0) return true;
			--blockcount;
			if (blockcount>0) {
				mode=10;
				oe.set_code_position(jmp2);
				goto next_block;
			}
			// unmatched
			oe.r=returnvalueflow::EXCEPTION;
			return false; }
		case 30:
			oe.exception_object_being_handled->gc_release(*oe.vm);
			oe.exception_object_being_handled=old_being_handled;
			old_being_handled=NULL;
			oe.vm->execution_exception_object_thrown=NULL;
			return set_dest_else_finally(oe,finallyblock);
		default:
			RCASSERT(0);
		}
		return false;
	}

	RCLMFUNCTION bool op_flow_try::resume_exception(vm_execution_stack_elem_internal &oe)
	{
		switch(mode) {
		case 0:
			mode=9;
			return resume_fin(oe);
		default:
			return false;
		}
	}

	RCLMFUNCTION bool op_try_flow(vm_execution_stack_elem_internal &oe)
	{
		op_flow_try *ff;
		oe.push(ff,0);

		RCASSERT(oe.tempstackactpos==0);

		oe >> ff->blocks >> ff->elseblock >> ff->finallyblock >> ff->done;
		ff->mode=0;
		ff->old_being_handled=NULL;
		return true;
	}

} }












