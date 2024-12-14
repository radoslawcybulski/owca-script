#include "stdafx.h"
#include "base.h"
#include "vm_execution_stack_elem_external.h"
#include "virtualmachine.h"
#include "exec_object.h"
#include "exec_function_ptr.h"
#include "exec_string.h"

namespace owca {
	namespace __owca__ {

		extern unsigned char operator_operand_count[];

		vm_execution_stack_elem_base::~vm_execution_stack_elem_base()
		{
			delete [] oper_2_values;
		}

		void vm_execution_stack_elem_base::_mark_gc(const gc_iteration &gc) const
		{
			if (init_object) {
				gc_iteration::debug_info _d("vm_execution_stack_elem_base %s: init object", fnc ? fnc->name()->data_pointer() : "<null>");
				init_object->gc_mark(gc);
			}
			if (fnc) {
				gc_iteration::debug_info _d("vm_execution_stack_elem_base %s: function object", fnc ? fnc->name()->data_pointer() : "<null>");
				fnc->gc_mark(gc);
			}
			if (oper_2_values) {
				unsigned int cnt=operator_operand_count[operator_index];
				switch(cnt) {
				case 4: {
					gc_iteration::debug_info _d("vm_execution_stack_elem_base %s: operator variable object 3", fnc ? fnc->name()->data_pointer() : "<null>");
					oper_2_values[3].gc_mark(gc); }
				case 3: {
					gc_iteration::debug_info _d("vm_execution_stack_elem_base %s: operator variable object 2", fnc ? fnc->name()->data_pointer() : "<null>");
					oper_2_values[2].gc_mark(gc); }
				case 2: {
					gc_iteration::debug_info _d("vm_execution_stack_elem_base %s: operator variable object 1", fnc ? fnc->name()->data_pointer() : "<null>");
					oper_2_values[1].gc_mark(gc); }
				case 1: {
					gc_iteration::debug_info _d("vm_execution_stack_elem_base %s: operator variable object 0", fnc ? fnc->name()->data_pointer() : "<null>");
					oper_2_values[0].gc_mark(gc); }
					break;
				default:
					RCASSERT(0);
				}
			}
		}

		void vm_execution_stack_elem_base::_release_resources(virtual_machine &vm)
		{
			if (init_object) init_object->gc_release(vm);
			if (fnc) fnc->gc_release(vm);
			if (oper_2_values) {
				unsigned int cnt=operator_operand_count[operator_index];
				switch(cnt) {
				case 4: oper_2_values[3].gc_release(vm);
				case 3: oper_2_values[2].gc_release(vm);
				case 2: oper_2_values[1].gc_release(vm);
				case 1: oper_2_values[0].gc_release(vm);
					break;
				default:
					RCASSERT(0);
				}
			}
		}

		void vm_execution_stack_elem_external_self__::_mark_gc(const gc_iteration &gc) const
		{
			{
				gc_iteration::debug_info _d("vm_execution_stack_elem_external_self__ %s: self object", fnc ? fnc->name()->data_pointer() : "<null>");
				v_self.gc_mark(gc);
			}
			vm_execution_stack_elem_external__::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_self__::_release_resources(virtual_machine &vm)
		{
			v_self.gc_release(vm);
			vm_execution_stack_elem_external__::_release_resources(vm);
		}

		void vm_execution_stack_elem_external_self_0::_mark_gc(const gc_iteration &gc) const
		{
			{
				gc_iteration::debug_info _d("vm_execution_stack_elem_external_self_0 %s: self object", fnc ? fnc->name()->data_pointer() : "<null>");
				v_self.gc_mark(gc);
			}
			vm_execution_stack_elem_external_0::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_self_1::_mark_gc(const gc_iteration &gc) const
		{
			{
				gc_iteration::debug_info _d("vm_execution_stack_elem_external_self_1 %s: self object", fnc ? fnc->name()->data_pointer() : "<null>");
				v_self.gc_mark(gc);
			}
			vm_execution_stack_elem_external_1::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_self_2::_mark_gc(const gc_iteration &gc) const
		{
			{
				gc_iteration::debug_info _d("vm_execution_stack_elem_external_self_2 %s: self object", fnc ? fnc->name()->data_pointer() : "<null>");
				v_self.gc_mark(gc);
			}
			vm_execution_stack_elem_external_2::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_self_3::_mark_gc(const gc_iteration &gc) const
		{
			{
				gc_iteration::debug_info _d("vm_execution_stack_elem_external_self_3 %s: self object", fnc ? fnc->name()->data_pointer() : "<null>");
				v_self.gc_mark(gc);
			}
			vm_execution_stack_elem_external_3::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_self_0::_release_resources(virtual_machine &vm)
		{
			v_self.gc_release(vm);
			vm_execution_stack_elem_external_0::_release_resources(vm);
		}

		void vm_execution_stack_elem_external_self_1::_release_resources(virtual_machine &vm)
		{
			v_self.gc_release(vm);
			vm_execution_stack_elem_external_1::_release_resources(vm);
		}

		void vm_execution_stack_elem_external_self_2::_release_resources(virtual_machine &vm)
		{
			v_self.gc_release(vm);
			vm_execution_stack_elem_external_2::_release_resources(vm);
		}

		void vm_execution_stack_elem_external_self_3::_release_resources(virtual_machine &vm)
		{
			v_self.gc_release(vm);
			vm_execution_stack_elem_external_3::_release_resources(vm);
		}


		void vm_execution_stack_elem_external__::_mark_gc(const gc_iteration &gc) const
		{
			vm_execution_stack_elem_base::_mark_gc(gc);
			if (mapobject) {
				gc_iteration::debug_info _d("vm_execution_stack_elem_external__ %s: map object", fnc ? fnc->name()->data_pointer() : "<null>");
				mapobject->gc_mark(gc);
			}
		}

		void vm_execution_stack_elem_external__::_release_resources(virtual_machine &vm)
		{
			vm_execution_stack_elem_base::_release_resources(vm);
			if (mapobject) mapobject->gc_release(vm);
		}

		void vm_execution_stack_elem_external_0::_mark_gc(const gc_iteration &gc) const
		{
			vm_execution_stack_elem_base::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_1::_mark_gc(const gc_iteration &gc) const
		{
			for(unsigned int i=0;i<paramcount;++i) {
				gc_iteration::debug_info _d("vm_execution_stack_elem_external_1 %s: param object %d", fnc ? fnc->name()->data_pointer() : "<null>", i);
				v_params[i].gc_mark(gc);
			}
			vm_execution_stack_elem_base::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_2::_mark_gc(const gc_iteration &gc) const
		{
			for(unsigned int i=0;i<paramcount;++i) {
				gc_iteration::debug_info _d("vm_execution_stack_elem_external_2 %s: param object %d", fnc ? fnc->name()->data_pointer() : "<null>", i);
				v_params[i].gc_mark(gc);
			}
			vm_execution_stack_elem_base::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_3::_mark_gc(const gc_iteration &gc) const
		{
			for(unsigned int i=0;i<paramcount;++i) {
				gc_iteration::debug_info _d("vm_execution_stack_elem_external_3 %s: param object %d", fnc ? fnc->name()->data_pointer() : "<null>", i);
				v_params[i].gc_mark(gc);
			}
			vm_execution_stack_elem_base::_mark_gc(gc);
		}

		void vm_execution_stack_elem_external_0::_release_resources(virtual_machine &vm)
		{
			vm_execution_stack_elem_base::_release_resources(vm);
		}

		void vm_execution_stack_elem_external_1::_release_resources(virtual_machine &vm)
		{
			for(unsigned int i=0;i<paramcount;++i) v_params[i].gc_release(vm);
			vm_execution_stack_elem_base::_release_resources(vm);
		}

		void vm_execution_stack_elem_external_2::_release_resources(virtual_machine &vm)
		{
			for(unsigned int i=0;i<paramcount;++i) v_params[i].gc_release(vm);
			vm_execution_stack_elem_base::_release_resources(vm);
		}

		void vm_execution_stack_elem_external_3::_release_resources(virtual_machine &vm)
		{
			for(unsigned int i=0;i<paramcount;++i) v_params[i].gc_release(vm);
			vm_execution_stack_elem_base::_release_resources(vm);
		}

		executionstackreturnvalue vm_execution_stack_elem_external_base::first_time_execute(executionstackreturnvalue mode)
		{
			first_time_run(false);
			switch(mode.type()) {
			case executionstackreturnvalue::EXCEPTION: return executionstackreturnvalue::EXCEPTION;
			case executionstackreturnvalue::OK: break;
			case executionstackreturnvalue::RETURN:
			case executionstackreturnvalue::FUNCTION_CALL:
			case executionstackreturnvalue::CREATE_GENERATOR:
				RCASSERT(0);
			default:
				RCASSERT(0);
			}
			if (!init()) {
				show_in_exception_stack(false);
				switch(return_handling_mode) {
				case RETURN_HANDLING_OPERATOR_RETURN_ANY_UPDATE_OPER:
				case RETURN_HANDLING_OPERATOR_RETURN_BOOL_UPDATE_OPER:
					return_value->set_null(true);
					return executionstackreturnvalue::RETURN;
				case RETURN_HANDLING_NONE:
				case RETURN_HANDLING_OPERATOR_RETURN_INIT:
				case RETURN_HANDLING_OPERATOR_RETURN_INIT_EXCEPTION:
				case RETURN_HANDLING_OPERATOR_RETURN_CANT_BE_UNUSED:
				case RETURN_HANDLING_OPERATOR_RETURN_UNUSED_UNSUPPORTED:
				case RETURN_HANDLING_OPERATOR_RETURN_BOOL:
				case RETURN_HANDLING_OPERATOR_RETURN_INT:
				case RETURN_HANDLING_OPERATOR_RETURN_STR:
					break;
				default:
					RCASSERT(0);
				}
				vm->raise_invalid_param(OWCA_ERROR_FORMAT1("cant call function %1 with given parameters",fnc->name()->str()));
				return executionstackreturnvalue::FUNCTION_CALL;
			}
			return execute(executionstackreturnvalue::OK);
		}

	}

	using namespace __owca__;

	owca_vm &owca_user_function_base_object::vm() const
	{
		return *vm_execution_stack_elem_external_base::vm->owner_vm;
	}

	void owca_user_function_base_object::_mark_gc(const gc_iteration &gc) const
	{
		self.gc_mark(gc);
		vm_execution_stack_elem_external_self__::_mark_gc(gc);
	}

	void owca_user_function_base_object::_release_resources(virtual_machine &vm)
	{
		self.null_set();
		vm_execution_stack_elem_external_self__::_release_resources(vm);
	}

	__owca__::executionstackreturnvalue owca_user_function_base_object::parse(owca_function_return_value frv)
	{
		switch(frv.type()) {
		case owca_function_return_value::EXCEPTION:
			vm_execution_stack_elem_external_self__::vm->_raise_from_user(tmp_return_value._object);
			return executionstackreturnvalue::EXCEPTION;
		case owca_function_return_value::RETURN_VALUE:
			*return_value = tmp_return_value._object;
			tmp_return_value._object.reset();
			return executionstackreturnvalue::RETURN;
		case owca_function_return_value::CREATE_GENERATOR:
			return executionstackreturnvalue::CREATE_GENERATOR;
		case owca_function_return_value::COROUTINE_STOP:
			return executionstackreturnvalue::CO_STOP;
		case owca_function_return_value::FUNCTION_CALL:
			return executionstackreturnvalue::FUNCTION_CALL;
		default:
			RCASSERT(0);
		}
		return_value->set_null(true);
		return executionstackreturnvalue::RETURN;
	}

	executionstackreturnvalue owca_user_function_base_object::first_time_execute(executionstackreturnvalue mode)
	{
		first_time_run(false);
		switch(mode.type()) {
		case executionstackreturnvalue::EXCEPTION: return executionstackreturnvalue::EXCEPTION;
		case executionstackreturnvalue::OK: break;
		case executionstackreturnvalue::RETURN:
		case executionstackreturnvalue::FUNCTION_CALL:
		case executionstackreturnvalue::CREATE_GENERATOR:
			RCASSERT(0);
		default:
			RCASSERT(0);
		}
		self._update_vm(vm_execution_stack_elem_external_self__::vm);
		self._object=v_self;
		v_self.gc_acquire();
		parameters._init(*vm_execution_stack_elem_external_self__::vm,cp);
		owca_function_return_value frv = initialize(tmp_return_value);
		return parse(frv);
	}

	executionstackreturnvalue owca_user_function_base_object::execute(executionstackreturnvalue r) {
		bool exc;
		if (r.type()==executionstackreturnvalue::EXCEPTION) {
			exc=true;
			tmp_return_value._object.gc_release(*vm_execution_stack_elem_external_self__::vm);
			tmp_return_value._update_vm(vm_execution_stack_elem_external_self__::vm);
			tmp_return_value._object.set_object(vm_execution_stack_elem_external_self__::vm->execution_exception_object_thrown);
			vm_execution_stack_elem_external_self__::vm->execution_exception_object_thrown=NULL;
		}
		else {
			RCASSERT(r.type()==executionstackreturnvalue::OK);
			exc=false;
		}
		owca_function_return_value frv = run(tmp_return_value,exc);
		return parse(frv);
	}
}

