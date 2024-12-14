#include "stdafx.h"
#include "debug_execution.h"
#include "debug_opcodes.h"
#include "base.h"
#include "exec_function_stack_data.h"
#include "vm_execution_stack_elem_internal.h"
#include "exec_string.h"
#include "exec_variablelocation.h"
#include "exec_stack.h"
#include "exec_variable.h"
#include "virtualmachine.h"
#include "exec_function_ptr.h"
#include "exec_object.h"
#include "op_validate.h"
#include "op_write.h"

namespace owca {
	namespace __owca__ {
		bool op_access_1_read_flow(vm_execution_stack_elem_internal &oe);
		bool op_access_1_write_flow(vm_execution_stack_elem_internal &oe);
		bool op_access_1_write_oper_flow(vm_execution_stack_elem_internal &oe);
		bool op_access_2_read_flow(vm_execution_stack_elem_internal &oe);
		bool op_access_2_write_flow(vm_execution_stack_elem_internal &oe);
		bool op_access_2_write_oper_flow(vm_execution_stack_elem_internal &oe);
		bool op_add_flow(vm_execution_stack_elem_internal &oe);
		bool op_assign_tuple_flow(vm_execution_stack_elem_internal &oe);
		bool op_bin_and_flow(vm_execution_stack_elem_internal &oe);
		bool op_bin_not_flow(vm_execution_stack_elem_internal &oe);
		bool op_bin_or_flow(vm_execution_stack_elem_internal &oe);
		bool op_bin_xor_flow(vm_execution_stack_elem_internal &oe);
		bool op_bool_flow(vm_execution_stack_elem_internal &oe);
		bool op_break_flow(vm_execution_stack_elem_internal &oe);
		bool op_compare_flow(vm_execution_stack_elem_internal &oe);
		bool op_compare_simple_flow(vm_execution_stack_elem_internal &oe);
		bool op_continue_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_array_comprehension_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_array_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_class_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_false_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_function_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_int_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_map_comprehension_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_map_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_null_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_real_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_set_comprehension_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_set_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_string_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_true_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_tuple_comprehension_flow(vm_execution_stack_elem_internal &oe);
		bool op_create_tuple_flow(vm_execution_stack_elem_internal &oe);
		bool op_div_flow(vm_execution_stack_elem_internal &oe);
		bool op_fin_1_clear_flow(vm_execution_stack_elem_internal &oe);
		bool op_fin_1_clear_for_flow(vm_execution_stack_elem_internal &oe);
		bool op_fin_flow(vm_execution_stack_elem_internal &oe);
		bool op_finally_flow(vm_execution_stack_elem_internal &oe);
		bool op_for_flow(vm_execution_stack_elem_internal &oe);
		bool op_function_call_flow(vm_execution_stack_elem_internal &oe);
		bool op_function_call_list_map_flow(vm_execution_stack_elem_internal &oe);
		bool op_generator_flow(vm_execution_stack_elem_internal &oe);
		bool op_ident_read_flow(vm_execution_stack_elem_internal &oe);
		bool op_ident_write_flow(vm_execution_stack_elem_internal &oe);
		bool op_ident_write_oper_flow(vm_execution_stack_elem_internal &oe);
		bool op_ident_write_property_flow(vm_execution_stack_elem_internal &oe);
		bool op_if_flow(vm_execution_stack_elem_internal &oe);
		bool op_in_flow(vm_execution_stack_elem_internal &oe);
		bool op_is_flow(vm_execution_stack_elem_internal &oe);
		bool op_log_and_flow(vm_execution_stack_elem_internal &oe);
		bool op_log_not_flow(vm_execution_stack_elem_internal &oe);
		bool op_log_or_flow(vm_execution_stack_elem_internal &oe);
		bool op_lookup_read_flow(vm_execution_stack_elem_internal &oe);
		bool op_lookup_write_flow(vm_execution_stack_elem_internal &oe);
		bool op_lookup_write_oper_flow(vm_execution_stack_elem_internal &oe);
		bool op_lshift_flow(vm_execution_stack_elem_internal &oe);
		bool op_mod_flow(vm_execution_stack_elem_internal &oe);
		bool op_mul_flow(vm_execution_stack_elem_internal &oe);
		bool op_noop_flow(vm_execution_stack_elem_internal &oe);
		bool op_que_flow(vm_execution_stack_elem_internal &oe);
		bool op_raise_flow(vm_execution_stack_elem_internal &oe);
		bool op_raise_no_value_flow(vm_execution_stack_elem_internal &oe);
		bool op_restart_flow(vm_execution_stack_elem_internal &oe);
		bool op_return_flow(vm_execution_stack_elem_internal &oe);
		bool op_return_no_value_flow(vm_execution_stack_elem_internal &oe);
		bool op_rshift_flow(vm_execution_stack_elem_internal &oe);
		bool op_sign_change_flow(vm_execution_stack_elem_internal &oe);
		bool op_sub_flow(vm_execution_stack_elem_internal &oe);
		bool op_try_flow(vm_execution_stack_elem_internal &oe);
		bool op_while_flow(vm_execution_stack_elem_internal &oe);
		bool op_with_flow(vm_execution_stack_elem_internal &oe);
		bool op_yield_flow(vm_execution_stack_elem_internal &oe);
	}
}

namespace owca {
	namespace __owca__ {

		typedef bool(*opcodefuncion)(vm_execution_stack_elem_internal &);

		static opcodefuncion opcodefunction_map[] = {
			/*  EXEC_COMPARE_SIMPLE           */ op_compare_simple_flow,
			/*  EXEC_COMPARE                  */ op_compare_flow,
			/*  EXEC_ADD                      */ op_add_flow,
			/*  EXEC_SUB                      */ op_sub_flow,
			/*  EXEC_MUL                      */ op_mul_flow,
			/*  EXEC_DIV                      */ op_div_flow,
			/*  EXEC_MOD                      */ op_mod_flow,
			/*  EXEC_LSHIFT                   */ op_lshift_flow,
			/*  EXEC_RSHIFT                   */ op_rshift_flow,
			/*  EXEC_BIN_AND                  */ op_bin_and_flow,
			/*  EXEC_BIN_OR                   */ op_bin_or_flow,
			/*  EXEC_BIN_XOR                  */ op_bin_xor_flow,
			/*  EXEC_ACCESS_1_READ            */ op_access_1_read_flow,
			/*  EXEC_ACCESS_1_WRITE           */ op_access_1_write_flow,
			/*  EXEC_ACCESS_1_WRITE_OPER      */ op_access_1_write_oper_flow,
			/*  EXEC_ACCESS_2_READ            */ op_access_2_read_flow,
			/*  EXEC_ACCESS_2_WRITE           */ op_access_2_write_flow,
			/*  EXEC_ACCESS_2_WRITE_OPER      */ op_access_2_write_oper_flow,
			/*  EXEC_BIN_NOT                  */ op_bin_not_flow,
			/*  EXEC_SIGN_CHANGE              */ op_sign_change_flow,
			/*  EXEC_ASSIGN_TUPLE             */ op_assign_tuple_flow,
			/*  EXEC_CREATE_TUPLE             */ op_create_tuple_flow,
			/*  EXEC_CREATE_ARRAY             */ op_create_array_flow,
			/*  EXEC_CREATE_MAP               */ op_create_map_flow,
			/*  EXEC_CREATE_SET               */ op_create_set_flow,
			/*  EXEC_CREATE_TUPLE_COMPREHENSION */ op_create_tuple_comprehension_flow,
			/*  EXEC_CREATE_ARRAY_COMPREHENSION */ op_create_array_comprehension_flow,
			/*  EXEC_CREATE_MAP_COMPREHENSION */ op_create_map_comprehension_flow,
			/*  EXEC_CREATE_SET_COMPREHENSION */ op_create_set_comprehension_flow,
			/*  EXEC_CREATE_INT               */ op_create_int_flow,
			/*  EXEC_CREATE_REAL              */ op_create_real_flow,
			/*  EXEC_CREATE_STRING            */ op_create_string_flow,
			/*  EXEC_CREATE_TRUE              */ op_create_true_flow,
			/*  EXEC_CREATE_FALSE             */ op_create_false_flow,
			/*  EXEC_CREATE_NULL              */ op_create_null_flow,
			/*  EXEC_CREATE_FUNCTION          */ op_create_function_flow,
			/*  EXEC_CREATE_CLASS             */ op_create_class_flow,
			/*  EXEC_LOOKUP_READ              */ op_lookup_read_flow,
			/*  EXEC_LOOKUP_WRITE             */ op_lookup_write_flow,
			/*  EXEC_LOOKUP_WRITE_OPER        */ op_lookup_write_oper_flow,
			/*  EXEC_IDENT_READ               */ op_ident_read_flow,
			/*  EXEC_IDENT_WRITE              */ op_ident_write_flow,
			/*  EXEC_IDENT_WRITE_OPER         */ op_ident_write_oper_flow,
			/*  EXEC_IDENT_WRITE_PROPERTY     */ op_ident_write_property_flow,
			/*  EXEC_FUNCTION_CALL            */ op_function_call_flow,
			/*  EXEC_FUNCTION_CALL_LIST_MAP   */ op_function_call_list_map_flow,
			/*  EXEC_LOG_AND                  */ op_log_and_flow,
			/*  EXEC_LOG_OR                   */ op_log_or_flow,
			/*  EXEC_LOG_NOT                  */ op_log_not_flow,
			/*  EXEC_IS                       */ op_is_flow,
			/*  EXEC_IN                       */ op_in_flow,
			/*  EXEC_BOOL                     */ op_bool_flow,
			/*  EXEC_GENERATOR                */ op_generator_flow,
			/*  EXEC_QUE                      */ op_que_flow,
			/*  EXEC_COUNT                    */ NULL,
			/*  FLOW_FOR                      */ op_for_flow,
			/*  FLOW_IF                       */ op_if_flow,
			/*  FLOW_NOOP                     */ op_noop_flow,
			/*  FLOW_RETURN                   */ op_return_flow,
			/*  FLOW_RETURN_NO_VALUE          */ op_return_no_value_flow,
			/*  FLOW_TRY                      */ op_try_flow,
			/*  FLOW_WHILE                    */ op_while_flow,
			/*  FLOW_WITH                     */ op_with_flow,
			/*  FLOW_BREAK                    */ op_break_flow,
			/*  FLOW_CONTINUE                 */ op_continue_flow,
			/*  FLOW_RESTART                  */ op_restart_flow,
			/*  FLOW_FINALLY                  */ op_finally_flow,
			/*  FLOW_YIELD                    */ op_yield_flow,
			/*  FLOW_RAISE                    */ op_raise_flow,
			/*  FLOW_RAISE_NO_VALUE           */ op_raise_no_value_flow,
			/*  FLOW_FIN                      */ op_fin_flow,
			/*  FLOW_FIN_1_CLEAR              */ op_fin_1_clear_flow,
			/*  FLOW_FIN_1_CLEAR_FOR          */ op_fin_1_clear_for_flow,
			/*  FLOW_COUNT                    */ NULL,
		};


#define R(a) (((a) + sizeof(unsigned int) - 1) & ~(sizeof(unsigned int) - 1))

		bool vm_execution_stack_elem_internal::peek_exec()
		{
			unsigned int off = opcodes_offset;
			ensure_type(OST_EXECOPCODE);
			if (opcodes->get_opcodes()[opcodes_offset] == EXEC_COUNT) {
				opcodes_offset += R(1);
				return false;
			}
			opcodes_offset = off;
			return true;
		}

#ifdef RCDEBUG_OPCODES
		void vm_execution_stack_elem_internal::ensure_type(opcodestreamtype type)
		{
			opcodestreamtype tp = (opcodestreamtype)opcodes->get_opcodes()[opcodes_offset];
			opcodes_offset += R(1);
			RCASSERT(tp == type);
		}
#endif
		void vm_execution_stack_elem_internal::_release_resources(virtual_machine &vm)
		{
			stack->gc_release(vm);
			fncstackdata->gc_release(vm);
			RCPRINTF("releaing %p\n", fncstackdata);
			for (unsigned int i = 0; i < tempstackactpos; ++i) temp(i).gc_release(vm);
			vm_execution_stack_elem_base::_release_resources(vm);
			if (exception_object_being_handled) exception_object_being_handled->gc_release(vm);
		}

		void vm_execution_stack_elem_internal::_mark_gc(const gc_iteration &gc) const
		{
			{
				gc_iteration::debug_info _d("vm_execution_stack_elem_internal %s: stack object", fnc ? fnc->name()->data_pointer() : "<null>");
				stack->gc_mark(gc);
			}
			{
				gc_iteration::debug_info _d("vm_execution_stack_elem_internal %s: function stack object", fnc ? fnc->name()->data_pointer() : "<null>");
				fncstackdata->gc_mark(gc);
			}
			for (unsigned int i = 0; i < tempstackactpos; ++i) {
				gc_iteration::debug_info _d("vm_execution_stack_elem_internal %s: tmp variable %d", fnc ? fnc->name()->data_pointer() : "<null>", i);
				temp(i).gc_mark(gc);
			}
			vm_execution_stack_elem_base::_mark_gc(gc);
			if (exception_object_being_handled) {
				gc_iteration::debug_info _d("vm_execution_stack_elem_internal %s: exc object being handled", fnc ? fnc->name()->data_pointer() : "<null>");
				exception_object_being_handled->gc_mark(gc);
			}
		}

		exec_variable &vm_execution_stack_elem_internal::get(const exec_variable_location &loc)
		{
			return stack->get_variable(loc);
		}

		exec_variable &vm_execution_stack_elem_internal::get0(unsigned int short vl)
		{
			return stack->get_0(vl);
		}

		exec_variable &vm_execution_stack_elem_internal::get_local(const exec_variable_location &loc)
		{
			RCASSERT(loc.depth() > 0);
			return stack->get_local(loc);
		}

		exec_variable &vm_execution_stack_elem_internal::temp(unsigned int index)
		{
			RCASSERT(index <= tempstackactpos);
			return ((exec_variable*)(((char*)this) + sizeof(*this)))[index];
		}

		const exec_variable &vm_execution_stack_elem_internal::temp(unsigned int index) const
		{
			RCASSERT(index <= tempstackactpos);
			return ((const exec_variable*)(((const char*)this) + sizeof(*this)))[index];
		}

		vm_execution_stack_elem_internal &vm_execution_stack_elem_internal::operator >> (operatorcodes &val)
		{
			ensure_type(OST_OPERATOROPCODE);
			val = (operatorcodes)opcodes->get_opcodes()[opcodes_offset];
			opcodes_offset += R(1);
			return *this;
		}

		vm_execution_stack_elem_internal &vm_execution_stack_elem_internal::operator >> (unsigned int &val)
		{
			ensure_type(OST_UINT32);
			val = *(unsigned int*)(opcodes->get_opcodes() + opcodes_offset);
			opcodes_offset += R(sizeof(unsigned int));
			return *this;
		}

		vm_execution_stack_elem_internal &vm_execution_stack_elem_internal::operator >> (vm_execution_stack_elem_internal_jump &val)
		{
			ensure_type(OST_JUMP);
			val.offset = *(unsigned int*)(opcodes->get_opcodes() + opcodes_offset);
			opcodes_offset += R(sizeof(unsigned int));
			return *this;
		}

		vm_execution_stack_elem_internal &vm_execution_stack_elem_internal::operator >> (owca_internal_string *&val)
		{
			ensure_type(OST_STRING);
			store_uint_or_ptr si;
			(*this) >> si;
			val = (owca_internal_string*)si.get_pointer();
			//val->gc_acquire();
			return *this;
		}

		vm_execution_stack_elem_internal &vm_execution_stack_elem_internal::operator >> (store_uint_or_ptr &val)
		{
			ensure_type(OST_UINT32_OR_PTR);
			val.init_ptr(const_cast<unsigned char*>(opcodes->get_opcodes()) + opcodes_offset);
			opcodes_offset += R(sizeof(store_uint_or_ptr::data_type));
			return *this;
		}

		vm_execution_stack_elem_internal &vm_execution_stack_elem_internal::operator >> (owca_int &val)
		{
			ensure_type(OST_INT);
			store_int si;
			si.init_ptr(const_cast<unsigned char*>(opcodes->get_opcodes()) + opcodes_offset);
			val = si.get_integer();
			opcodes_offset += R(sizeof(store_int::data_type));
			return *this;
		}

		vm_execution_stack_elem_internal &vm_execution_stack_elem_internal::operator >> (owca_real &val)
		{
			ensure_type(OST_REAL);
			store_real si;
			si.init_ptr(const_cast<unsigned char*>(opcodes->get_opcodes()) + opcodes_offset);
			val = si.get_real();
			opcodes_offset += R(sizeof(store_real::data_type));
			return *this;
		}

		vm_execution_stack_elem_internal &vm_execution_stack_elem_internal::operator >> (exec_variable_location &vl)
		{
			ensure_type(OST_VARIABLELOCATION);
			store_variable_location svl;
			svl.init_ptr(const_cast<unsigned char*>(opcodes->get_opcodes()) + opcodes_offset);
			//svl.init_ptr(&)
			vl = exec_variable_location(svl.get_offset(), svl.get_depth());
			opcodes_offset += R(sizeof(store_variable_location::data_type));
			return *this;
		}

		extern unsigned char operator_operand_count[];


		struct keyword_param_iterator_lm : public virtual_machine::keyword_param_iterator {
			vm_execution_stack_elem_internal *oe;
			const exec_variable *params;
			unsigned int cnt, index;

			bool next(owca_internal_string *&id, const exec_variable *&val)
			{
				if (index >= cnt) return false;
				(*oe) >> id;
				val = &params[index];
				++index;
				return true;
			}

			unsigned int count() { return cnt; }

			keyword_param_iterator_lm() : index(0) { }
		};

		void vm_execution_stack_elem_internal::prepare_call_function_stack(unsigned int cnt1, unsigned int cnt2, unsigned char mode)
		{
			unsigned int total = cnt1 + cnt2 + (mode & 1 ? 1 : 0) + (mode & 2 ? 1 : 0) + (mode & 4 ? 1 : 0);
			exec_variable *ptr = &temp(tempstackactpos - total);

			const exec_variable &fnc = ptr[0];
			const exec_variable *normal_params = ptr + 1;
			unsigned int normal_params_count = cnt1 - 1;

			keyword_param_iterator_lm kpi;

			kpi.cnt = cnt2;
			kpi.oe = this;
			kpi.params = &ptr[cnt1];

			const exec_variable *spec = &ptr[cnt1 + cnt2];
			const exec_variable *listvar = mode & 1 ? spec++ : NULL;
			const exec_variable *mapvar = mode & 2 ? spec++ : NULL;
			const exec_variable *selfoverride = mode & 4 ? spec : NULL;

			// bool prepare_call_function(exec_variable *retval, const exec_variable &fnc,
			// const exec_variable *listvar, const exec_variable *mapvar, keyword_param_iterator &kpi,
			// const exec_variable *params, unsigned int paramcnt, const exec_variable *self_override);
			vm->prepare_call_function(ptr, fnc, listvar, mapvar, kpi, normal_params, normal_params_count, selfoverride);
			tempparamstrip = total - 1;
			ptr[0].gc_release(*vm);
			ptr[0].setmode(VAR_NULL);
		}

		void vm_execution_stack_elem_internal::prepare_call_function_stack(unsigned int paramcnt)
		{
			exec_variable *params = &temp(tempstackactpos - paramcnt);
			vm->prepare_call_function(params, params[0], params + 1, paramcnt - 1);
			tempparamstrip = paramcnt - 1;
			params[0].gc_release(*vm);
			params[0].setmode(VAR_NULL);
		}

		void vm_execution_stack_elem_internal::prepare_call_operator_stack(operatorcodes oper)
		{
			RCASSERT(operator_operand_count[oper] != 0);
			unsigned int opercnt = operator_operand_count[oper];
			RCASSERT(opercnt != 0);
			exec_variable *params = &temp(tempstackactpos - opercnt);
			switch (oper) {
			case E_ACCESS_1_WRITE: {
									   exec_variable tmp = params[0];
									   params[0] = params[1];
									   params[1] = params[2];
									   params[2] = tmp;
									   break; }
			case E_ACCESS_2_WRITE: {
									   exec_variable tmp = params[0];
									   params[0] = params[1];
									   params[1] = params[2];
									   params[2] = params[3];
									   params[3] = tmp;
									   break; }
			default:
				break;
			}
			vm->prepare_call_operator(params, oper, params);
			tempparamstrip = opercnt - 1;
			params[0].gc_release(*vm);
			params[0].setmode(VAR_NULL);
		}

		executionstackreturnvalue vm_execution_stack_elem_internal::first_time_execute(executionstackreturnvalue mode)
		{
			first_time_run(false);
			switch (mode.type()) {
			case executionstackreturnvalue::EXCEPTION: return executionstackreturnvalue::EXCEPTION;
			case executionstackreturnvalue::OK: break;
			case executionstackreturnvalue::RETURN:
			case executionstackreturnvalue::FUNCTION_CALL:
			case executionstackreturnvalue::CREATE_GENERATOR:
				RCASSERT(0);
			default:
				RCASSERT(0);
			}
			if (fnc != NULL && fnc->internal_is_generator()) {
				return executionstackreturnvalue::CREATE_GENERATOR;
			}
			return vm_execution_stack_elem_internal::execute(executionstackreturnvalue::OK);
		}

		RCLMFUNCTION executionstackreturnvalue vm_execution_stack_elem_internal::execute(executionstackreturnvalue mode)
		{
			switch (mode.type()) {
			case executionstackreturnvalue::OK:
				if (tempparamstrip) {
					exec_variable *params = &temp(tempstackactpos - tempparamstrip);
					for (unsigned int i = tempparamstrip; i > 0; --i) params[i - 1].gc_release(*vm);
					tempstackactpos -= tempparamstrip;
					tempparamstrip = 0;
				}
				break;
			case executionstackreturnvalue::EXCEPTION:
				r = returnvalueflow::EXCEPTION;
				break;
			case executionstackreturnvalue::RETURN:
				RCASSERT(0);
			default:
				RCASSERT(0);
			}

			for (;;) {
#ifdef RCDEBUG
				static unsigned int _cnt;
				unsigned int _cnt_self = ++_cnt;
				if (_cnt_self >= 9) {
					_cnt_self = _cnt_self;
				}
				owca_location location;
				location = actual_location();
#endif
				switch (r.type()) {
				case returnvalueflow::CONTINUE_OPCODES:
					if (!is_done()) {
						execopcode index;
						for (;;) {
#ifdef RCDEBUG
							char dbuf[256];
							_cnt_self = ++_cnt;
							RCASSERT(vm->execution_stack->peek_frame() == this);
#ifdef RCDEBUG_EXECUTION
							sprintf(dbuf, "%6d %4d", opcodes_offset, _cnt_self);
							debugprint(dbuf);
#endif
#endif
							ensure_type(OST_EXECOPCODE);
#ifdef RCDEBUG
							location = actual_location();
#ifdef RCDEBUG_EXECUTION
							sprintf(dbuf, " %s %4d %5d  ", location.filename(), location.line(), get_debug_ident());
							debugprint(dbuf);
#endif
#endif
							index = (execopcode)opcodes->get_opcodes()[opcodes_offset];
							opcodes_offset += R(1);
#ifdef RCDEBUG_EXECUTION
							sprintf(dbuf, "%s %d\n", execopcode_name(index), tempstackactpos);
							debugprint(dbuf);
#endif
							RCASSERT(index < FLOW_COUNT);
							if (!opcodefunction_map[index](*this)) break;
							GC(vm);
						}
					}
					else {
						return_value->set_null(true);
						return executionstackreturnvalue::RETURN;
					}
					break;
				case returnvalueflow::CALL_FUNCTION_CONTINUE_OPCODES:
					r = returnvalueflow::CONTINUE_OPCODES;
					return executionstackreturnvalue::FUNCTION_CALL;
				case returnvalueflow::CALL_FUNCTION_CONTINUE_FIN:
					r = returnvalueflow::FIN;
					return executionstackreturnvalue::FUNCTION_CALL;
				case returnvalueflow::YIELD:
					r = returnvalueflow::CONTINUE_OPCODES;
					return executionstackreturnvalue::RETURN;
				case returnvalueflow::RETURN:
					if (fncstackdata->empty()) {
						is_done(true);
						return executionstackreturnvalue::RETURN;
					}
					if (!fncstackdata->peek()->resume_return(*this)) fncstackdata->pop(*vm);
					break;
				case returnvalueflow::RETURN_NO_VALUE:
					if (fncstackdata->empty()) {
						is_done(true);
						return_value->set_null(true);
						return executionstackreturnvalue::RETURN;
					}
					if (!fncstackdata->peek()->resume_return(*this)) fncstackdata->pop(*vm);
					break;
				case returnvalueflow::FIN:
					RCASSERT(!fncstackdata->empty());
					if (!fncstackdata->peek()->resume_fin(*this)) fncstackdata->pop(*vm);
					break;
				case returnvalueflow::LOOP_CONTROL:
					RCASSERT(!fncstackdata->empty());
					if (!fncstackdata->peek()->resume_loop_control(*this)) fncstackdata->pop(*vm);
					break;
				case returnvalueflow::EXCEPTION:
					for (unsigned int i = 0; i < tempstackactpos; ++i) temp(i).gc_release(*vm);
					tempparamstrip = tempstackactpos = 0;
					if (fncstackdata->empty()) {
						is_done(true);
						return executionstackreturnvalue::EXCEPTION;
					}
					else {
						if (!fncstackdata->peek()->resume_exception(*this)) fncstackdata->pop(*vm);
					}
					break;
				default:
					RCASSERT(0);
				}
			}
		}
#undef R

		void vm_execution_stack_elem_internal::pop(op_flow_data_object *a)
		{
			RCASSERT(fncstackdata->peek() == a);
			fncstackdata->pop(*vm);
		}

		op_flow_data_object *vm_execution_stack_elem_internal::peek()
		{
			return fncstackdata->peek();
		}

		unsigned int vm_execution_stack_elem_internal::calculate_size(unsigned int tempvarcount)
		{
			return sizeof(vm_execution_stack_elem_internal)+sizeof(exec_variable)*tempvarcount;
		}

		owca_location vm_execution_stack_elem_internal::actual_location() const
		{
			return opcodes->get_location_from_opcode_index(opcodes_offset);
			//return owca_location(opcodes->_get_line_from_code_index(opcodes_offset),opcodes->filename().c_str());
		}
	}
}


