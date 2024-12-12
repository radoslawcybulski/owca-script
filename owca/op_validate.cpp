#include "stdafx.h"
#include "debug_execution.h"
#include "debug_opcodes.h"
#include "base.h"
#include "op_base.h"
#include "op_validate.h"
#include "op_write.h"
#include "exec_variablelocation.h"
#include "exec_string.h"
#include "vm_execution_stack_elem_internal.h"
#include "operatorcodes.h"
#include "virtualmachine.h"
#include <memory.h>

namespace owca {
	namespace __owca__ {
		opcode_validator::boolean_result op_access_1_read_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_access_1_write_oper_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_access_1_write_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_access_2_read_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_access_2_write_oper_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_access_2_write_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_add_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_assign_tuple_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_bin_and_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_bin_not_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_bin_or_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_bin_xor_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_bool_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_break_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_compare_simple_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_compare_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_continue_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_array_comprehension_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_array_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_class_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_false_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_function_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_int_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_map_comprehension_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_map_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_null_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_real_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_set_comprehension_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_set_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_string_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_true_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_tuple_comprehension_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_create_tuple_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_div_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_fin_1_clear_for_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_fin_1_clear_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_fin_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_finally_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_for_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_function_call_list_map_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_function_call_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_generator_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_ident_read_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_ident_write_oper_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_ident_write_property_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_ident_write_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_if_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_in_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_is_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_log_and_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_log_not_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_log_or_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_lookup_read_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_lookup_write_oper_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_lookup_write_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_lshift_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_mod_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_mul_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_noop_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_que_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_raise_no_value_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_raise_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_restart_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_return_no_value_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_return_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_rshift_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_sign_change_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_sub_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_try_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_while_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_with_validate(opcode_validator &oe);
		opcode_validator::boolean_result op_yield_validate(opcode_validator &oe);
	}
}

namespace owca {
	namespace __owca__ {


		owca_location opcode_data::get_location_from_opcode_index(unsigned int opcode_index) const
		{
			for (auto a : location_infos) {
				if (opcode_index >= a.offset_begin && opcode_index <= a.offset_end)
                    return owca_location(a.line, file_name.c_str());
			}

			RCASSERT(0);
			return owca_location();
		}

		void opcode_data::gc_mark(const gc_iteration &gi) const
		{
			//for (auto a : strings)
			//	a->gc_mark(gi);
		}
		void opcode_data::gc_release(virtual_machine &vm)
		{
			//for (auto &a : strings) {
			//	a->gc_release(vm);
			//	a = NULL;
			//}
			strings.clear();
		}


		typedef opcode_validator::boolean_result(*opcodefunc_validate)(opcode_validator &oe);
		static opcodefunc_validate opcode_validate_map[] = {
			/*  EXEC_COMPARE_SIMPLE           */ op_compare_simple_validate,
			/*  EXEC_COMPARE                  */ op_compare_validate,
			/*  EXEC_ADD                      */ op_add_validate,
			/*  EXEC_SUB                      */ op_sub_validate,
			/*  EXEC_MUL                      */ op_mul_validate,
			/*  EXEC_DIV                      */ op_div_validate,
			/*  EXEC_MOD                      */ op_mod_validate,
			/*  EXEC_LSHIFT                   */ op_lshift_validate,
			/*  EXEC_RSHIFT                   */ op_rshift_validate,
			/*  EXEC_BIN_AND                  */ op_bin_and_validate,
			/*  EXEC_BIN_OR                   */ op_bin_or_validate,
			/*  EXEC_BIN_XOR                  */ op_bin_xor_validate,
			/*  EXEC_ACCESS_1_READ            */ op_access_1_read_validate,
			/*  EXEC_ACCESS_1_WRITE           */ op_access_1_write_validate,
			/*  EXEC_ACCESS_1_WRITE_OPER      */ op_access_1_write_oper_validate,
			/*  EXEC_ACCESS_2_READ            */ op_access_2_read_validate,
			/*  EXEC_ACCESS_2_WRITE           */ op_access_2_write_validate,
			/*  EXEC_ACCESS_2_WRITE_OPER      */ op_access_2_write_oper_validate,
			/*  EXEC_BIN_NOT                  */ op_bin_not_validate,
			/*  EXEC_SIGN_CHANGE              */ op_sign_change_validate,
			/*  EXEC_ASSIGN_TUPLE             */ op_assign_tuple_validate,
			/*  EXEC_CREATE_TUPLE             */ op_create_tuple_validate,
			/*  EXEC_CREATE_ARRAY             */ op_create_array_validate,
			/*  EXEC_CREATE_MAP               */ op_create_map_validate,
			/*  EXEC_CREATE_SET               */ op_create_set_validate,
			/*  EXEC_CREATE_TUPLE_COMPREHENSION */ op_create_tuple_comprehension_validate,
			/*  EXEC_CREATE_ARRAY_COMPREHENSION */ op_create_array_comprehension_validate,
			/*  EXEC_CREATE_MAP_COMPREHENSION */ op_create_map_comprehension_validate,
			/*  EXEC_CREATE_SET_COMPREHENSION */ op_create_set_comprehension_validate,
			/*  EXEC_CREATE_INT               */ op_create_int_validate,
			/*  EXEC_CREATE_REAL              */ op_create_real_validate,
			/*  EXEC_CREATE_STRING            */ op_create_string_validate,
			/*  EXEC_CREATE_TRUE              */ op_create_true_validate,
			/*  EXEC_CREATE_FALSE             */ op_create_false_validate,
			/*  EXEC_CREATE_NULL              */ op_create_null_validate,
			/*  EXEC_CREATE_FUNCTION          */ op_create_function_validate,
			/*  EXEC_CREATE_CLASS             */ op_create_class_validate,
			/*  EXEC_LOOKUP_READ              */ op_lookup_read_validate,
			/*  EXEC_LOOKUP_WRITE             */ op_lookup_write_validate,
			/*  EXEC_LOOKUP_WRITE_OPER        */ op_lookup_write_oper_validate,
			/*  EXEC_IDENT_READ               */ op_ident_read_validate,
			/*  EXEC_IDENT_WRITE              */ op_ident_write_validate,
			/*  EXEC_IDENT_WRITE_OPER         */ op_ident_write_oper_validate,
			/*  EXEC_IDENT_WRITE_PROPERTY     */ op_ident_write_property_validate,
			/*  EXEC_FUNCTION_CALL            */ op_function_call_validate,
			/*  EXEC_FUNCTION_CALL_LIST_MAP   */ op_function_call_list_map_validate,
			/*  EXEC_LOG_AND                  */ op_log_and_validate,
			/*  EXEC_LOG_OR                   */ op_log_or_validate,
			/*  EXEC_LOG_NOT                  */ op_log_not_validate,
			/*  EXEC_IS                       */ op_is_validate,
			/*  EXEC_IN                       */ op_in_validate,
			/*  EXEC_BOOL                     */ op_bool_validate,
			/*  EXEC_GENERATOR                */ op_generator_validate,
			/*  EXEC_QUE                      */ op_que_validate,
			/*  EXEC_COUNT                    */ NULL,
			/*  FLOW_FOR                      */ op_for_validate,
			/*  FLOW_IF                       */ op_if_validate,
			/*  FLOW_NOOP                     */ op_noop_validate,
			/*  FLOW_RETURN                   */ op_return_validate,
			/*  FLOW_RETURN_NO_VALUE          */ op_return_no_value_validate,
			/*  FLOW_TRY                      */ op_try_validate,
			/*  FLOW_WHILE                    */ op_while_validate,
			/*  FLOW_WITH                     */ op_with_validate,
			/*  FLOW_BREAK                    */ op_break_validate,
			/*  FLOW_CONTINUE                 */ op_continue_validate,
			/*  FLOW_RESTART                  */ op_restart_validate,
			/*  FLOW_FINALLY                  */ op_finally_validate,
			/*  FLOW_YIELD                    */ op_yield_validate,
			/*  FLOW_RAISE                    */ op_raise_validate,
			/*  FLOW_RAISE_NO_VALUE           */ op_raise_no_value_validate,
			/*  FLOW_FIN                      */ op_fin_validate,
			/*  FLOW_FIN_1_CLEAR              */ op_fin_1_clear_validate,
			/*  FLOW_FIN_1_CLEAR_FOR          */ op_fin_1_clear_for_validate,
			/*  FLOW_COUNT                    */ NULL,
		};

		opcode_validator::boolean_result opcode_validator::validate_read_expr()
		{
			unsigned int cnt = temporary_variables_count();
			do {
				execution_can_continue = continue_opcodes = true;
				if (!validate()) return false;
			} while (continue_opcodes);
			if (lastopcode != FLOW_FIN) return false;
			if (temporary_variables_count() != cnt + 1) return false;
			pop_temporary_variable();
			return true;
		}

		opcode_validator::boolean_result opcode_validator::validate_write_expr_que()
		{
			unsigned int cnt = temporary_variables_count();
			if (cnt == 0) return false;
			do {
				execution_can_continue = continue_opcodes = true;
				if (!validate()) return false;
			} while (continue_opcodes);
			if (lastopcode != FLOW_FIN) return false;
			if (temporary_variables_count() != cnt) return false;
			return true;
		}

		opcode_validator::boolean_result opcode_validator::validate_write_expr_assign_tuple()
		{
			if (temporary_variables_count() != 1) return false;
			do {
				execution_can_continue = continue_opcodes = true;
				if (!validate()) return false;
			} while (continue_opcodes);
			if (lastopcode != FLOW_FIN) return false;
			if (temporary_variables_count() != 1) return false;
			//pop_temporary_variable();
			return true;
		}

		opcode_validator::boolean_result opcode_validator::validate_write_expr_for()
		{
			if (temporary_variables_count() != 1) return false;
			do {
				execution_can_continue = continue_opcodes = true;
				if (!validate()) return false;
			} while (continue_opcodes);
			if (lastopcode != FLOW_FIN_1_CLEAR_FOR) return false;
			if (temporary_variables_count() != 0) return false;
			//pop_temporary_variable();
			return true;
		}

#define R(a) (((a) + sizeof(unsigned int) - 1) & ~(sizeof(unsigned int) - 1))
		opcode_validator::boolean_result opcode_validator::validate()
		{
#ifdef RCDEBUG
			static unsigned int execindex = 0;
			unsigned int actexecindex = ++execindex;

			if (actexecindex >= 17) {
				actexecindex = actexecindex;
			}
#endif
			ensure_type(OST_EXECOPCODE);
			if (offset >= data.size()) {
				return false;
			}
			lastopcode = (execopcode)data[offset];
			setdebugcodemap(offset, offset, "opcode %s", execopcode_name(lastopcode));
			offset += R(1);
#ifdef RCDEBUG
			execopcode lo = lastopcode;
			if (actexecindex >= 17) {
				actexecindex = actexecindex;
			}
#endif
			opcode_validator::boolean_result b = opcode_validate_map[lastopcode](*this);
			return b;
		}
	}
}

namespace owca {
	namespace __owca__ {
		opcode_validator::opcode_validator(virtual_machine &vm_, const std::vector<unsigned char> &data_, opcode_data *result_) : vm(vm_), data(data_), result(result_)
		{
			offset = 16;
		}

#ifdef RCDEBUG_OPCODES
		void opcode_validator::ensure_type(opcodestreamtype type)
		{
			RCASSERT(offset < data.size());
			opcodestreamtype ost = (opcodestreamtype)data[offset];
			RCASSERT(ost == type);
			unsigned int o = R(1);
			offset += o;
			ost = ost;
		}
#endif

		opcode_validator::boolean_result opcode_validator::get(owca_internal_string *&val)
		{
			ensure_type(OST_STRING);
			unsigned int off = offset;
			store_uint_or_ptr tmp;
			if (!get(tmp))
				return false;

			unsigned int index = tmp.get_uint();
			if (index >= strings.size())
				return false;

			val = strings[index];
			tmp.set_pointer(val);
			//val->value=val->data_pointer();
			setdebugcodemap(off, offset, "string (%d) '%s'", index, val->str().c_str());
			return true;
		}

		opcode_validator::boolean_result opcode_validator::get(exec_variable_location &vl)
		{
			ensure_type(OST_VARIABLELOCATION);
			unsigned int off = offset;
			if (offset + sizeof(store_variable_location::data_type) > data.size())
				return false;
			store_variable_location svl;
			svl.init_ptr(&data[offset]);
			offset += R(sizeof(store_variable_location::data_type));

			vl = exec_variable_location(svl.get_offset(),svl.get_depth());

			if (vl.valid()) setdebugcodemap(off, offset, "varloc depth=%2d offset=%3d", vl.depth(), vl.offset());
			else setdebugcodemap(off, offset, "varloc invalid");

			return true;
		}

		opcode_validator::boolean_result opcode_validator::get(owca_int &val)
		{
			ensure_type(OST_INT);
			unsigned int off = offset;
			if (offset + sizeof(store_int::data_type) > data.size()) return false;
			store_int si;
			si.init_ptr(&data[offset]);
			offset += R(sizeof(store_int::data_type));
			val = si.get_integer();
			setdebugcodemap(off, offset, "int    %d", (int)val);
			return true;
		}

		opcode_validator::boolean_result opcode_validator::get(owca_real &val)
		{
			ensure_type(OST_REAL);
			unsigned int off = offset;
			if (offset + sizeof(store_real::data_type) > data.size()) return false;
			store_real si;
			si.init_ptr(&data[offset]);
			offset += R(sizeof(store_real::data_type));
			val = si.get_real();
			setdebugcodemap(off, offset, "real   %f", (double)val);
			return true;
		}

		opcode_validator::boolean_result opcode_validator::get(store_uint_or_ptr &val)
		{
			ensure_type(OST_UINT32_OR_PTR);
			unsigned int off = offset;
			if (offset + sizeof(store_uint_or_ptr::data_type) > data.size()) return false;
			val.init_ptr(&data[offset]);
			offset += R(sizeof(store_uint_or_ptr::data_type));
			setdebugcodemap(off, offset, "uint_or_ptr %d", val.get_uint());
			return true;
		}

		opcode_validator::boolean_result opcode_validator::get(operatorcodes &opc)
		{
			ensure_type(OST_OPERATOROPCODE);
			unsigned int off = offset;
			if (offset + 1 > data.size()) return false;
			opc = (operatorcodes)data[offset];
			offset += R(1);
			setdebugcodemap(off, offset, "oper   %s", operatorcodes_name(opc));
			return true;
		}

		opcode_validator::boolean_result opcode_validator::get(unsigned int &val)
		{
			ensure_type(OST_UINT32);
			unsigned int off = offset;
			if (offset + sizeof(val) > data.size()) return false;
			val = *(unsigned int*)&data[offset];
			unsigned int o = R(sizeof(unsigned int));
			offset += R(sizeof(unsigned int));
			setdebugcodemap(off, offset, "uint   %d", val);
			return true;
		}

		opcode_validator::boolean_result opcode_validator::get(opcode_executer_jump &val)
		{
			ensure_type(OST_JUMP);
			unsigned int off = offset;
			if (offset + sizeof(unsigned int) > data.size())
				return false;
			unsigned int &p = *(unsigned int*)&data[offset];
			if (p + begin_code_ofset >= data.size())
				return false;
			p += offset - begin_code_ofset;
			val.offset = p;
			offset += R(sizeof(unsigned int));
			setdebugcodemap(off, offset, "jump   %u", val.offset);
			return true;
		}

		opcode_validator::boolean_result opcode_validator::check(unsigned int varoff)
		{
			return check(exec_variable_location(varoff, 0));
		}

		void opcode_validator::push_stack_size(unsigned int max_local_variables, unsigned int max_temporary_variables, unsigned int max_local_stack_data_size)
		{
			stacksizes.push_back(stacksizeinfo(max_local_variables, max_temporary_variables, max_local_stack_data_size));
		}

		opcode_validator::boolean_result opcode_validator::pop_stack_size()
		{
			stacksizeinfo &ss = stacksizes.back();
			if (ss.current != ss.maximum) {
				return false;
			}
			if (ss.tempcurrent != 0) {
				return false;
			}
			if (ss.tempmaximum != ss.tempmaximumfinal) {
				return false;
			}
			if (ss.stackyieldcurrent != 0) {
				return false;
			}
			if (ss.stackyieldmaximum != ss.stackyieldmaximumfinal) {
				return false;
			}
			stacksizes.pop_back();
			return true;
		}

		opcode_validator::boolean_result opcode_validator::push_local_stack_data_size(unsigned int sz)
		{
			stacksizeinfo &ss = stacksizes.back();
			if (ss.stackyieldcurrent + sz > ss.stackyieldmaximumfinal) return false;
			ss.stackyieldcurrent += sz;
			if (ss.stackyieldcurrent > ss.stackyieldmaximum) ss.stackyieldmaximum = ss.stackyieldcurrent;
			return true;
		}

		opcode_validator::boolean_result opcode_validator::pop_local_stack_data_size(unsigned int sz)
		{
			stacksizeinfo &ss = stacksizes.back();
			if (ss.stackyieldcurrent < sz) {
				RCASSERT(0);
				return false;
			}
			ss.stackyieldcurrent -= sz;
			return true;
		}

		opcode_validator::boolean_result opcode_validator::pop_temporary_variable()
		{
			unsigned int &temporary_variables = stacksizes.back().tempcurrent;
			if (temporary_variables == 0) return false;
			--temporary_variables;
			return true;
		}

		opcode_validator::boolean_result opcode_validator::pop_temporary_variables(unsigned int cnt)
		{
			unsigned int &temporary_variables = stacksizes.back().tempcurrent;
			if (temporary_variables < cnt) return false;
			temporary_variables -= cnt;
			return true;
		}

		opcode_validator::boolean_result opcode_validator::push_temporary_variable()
		{
			unsigned int &temporary_variables = stacksizes.back().tempcurrent;
			unsigned int &max_temporary_variables = stacksizes.back().tempmaximum;

			++temporary_variables;
			if (temporary_variables > max_temporary_variables) {
				max_temporary_variables = temporary_variables;
				if (max_temporary_variables > stacksizes.back().tempmaximumfinal) return false;
			}
			return true;
		}

		opcode_validator::boolean_result opcode_validator::check(const exec_variable_location &varloc)
		{
			if (varloc.depth() >= stacksizes.size()) return false;
			unsigned int index = 0;
			for (std::vector<stacksizeinfo>::iterator it = stacksizes.begin(); it != stacksizes.end(); ++it, ++index) if (index == varloc.depth()) {
				if (varloc.offset() >= it->maximum) return false;
				if (varloc.offset() >= it->current) it->current = varloc.offset() + 1;
				return true;
			}
			return false;
		}

		opcode_validator::boolean_result opcode_validator::peek_exec(bool &ret)
		{
			ensure_type(OST_EXECOPCODE);
			if (offset >= data.size()) return false;
			unsigned int off = offset;
			if (data[offset] == EXEC_COUNT) {
				offset += R(1);
				ret = false;
				return true;
			}
			offset = off;
			ret = true;
			return validate_read_expr();
		}

		opcode_validator::boolean_result opcode_validator::get_operands(unsigned int count)
		{
			RCASSERT(count > 0);
			unsigned int &temporary_variables = stacksizes.back().tempcurrent;
			if (temporary_variables < count) return false;
			temporary_variables -= count;
			temporary_variables += 1;
			return true;
		}

		unsigned int opcode_validator::temporary_variables_count() const
		{
			return stacksizes.back().tempcurrent;
		}

		//unsigned int opcode_validator::max_temporary_variables_count() const
		//{
		//	return max_temporary_variables;
		//}

		opcode_validator::boolean_result opcode_validator::validate_flow()
		{
			if (temporary_variables_count() != 0) {
				return false;
			}
			do {
				execution_can_continue = continue_opcodes = true;
				if (!validate()) {
					return false;
				}
			} while (continue_opcodes);
			if (temporary_variables_count() != 0) {
				return false;
			}
			return true;
		}

		owca_internal_string *opcode_validator::check_string(void)
		{
			unsigned int str_size = 0;
			if (!get(str_size))
				return NULL;

			if (offset + str_size > data.size())
				return NULL;

			ensure_type(OST_STRING_BODY);

			const char *p = (char*)&data[offset];
			offset += R(str_size);

			std::string z(p, str_size);
			auto it = vm.strings_used_by_opcode_datas.find(z);
			if (it == vm.strings_used_by_opcode_datas.end()) {
				unsigned int index = 0;
				unsigned int char_count = owca_internal_string::calculate_char_count_and_missing_bytes_if_any(index, p, str_size);
				RCASSERT(index == 0);
				owca_internal_string *s = vm.allocate_string(p, str_size, char_count);
#ifdef RCDEBUG
				s->value = s->data_pointer();
#endif
				it = vm.strings_used_by_opcode_datas.insert(std::pair<std::string,owca_internal_string*>(z,s)).first;
			}
			return it->second;
		}

		opcode_validator::boolean_result opcode_validator::validate(virtual_machine &vm, const std::vector<unsigned char> &opcodes, opcode_data *result)
		{
			//if (opcodes.size()<=sizeof(unsigned int)*5) return false;
			if (opcodes.size() < 16)
				return false;
			if (opcodes[0] != 'O' || opcodes[1] != 'W' || opcodes[ 2] != 'C' || opcodes[ 3] != 'A' ||
				opcodes[4] != 'S' || opcodes[5] != 'C' || opcodes[ 6] != 'R' || opcodes[ 7] != 'I' ||
				opcodes[8] != 'P' || opcodes[9] != 'T' || opcodes[10] != '!' || opcodes[11] != '\0')
				return false; // missing magic
			if (*(uint32_t*)&opcodes[12] != VERSION_MAGIC)
				return false; // wrong version

			opcode_validator ov(vm, opcodes, result);
			const char *tmp = (const char*)&ov.data[0];

			{
				unsigned int loc_info_count;

				if (!ov.get(loc_info_count) || loc_info_count * sizeof(vm_execution_stack_elem_internal::locationinfo) > opcodes.size())
					return false;

				ov.location_infos.resize(loc_info_count);
				for (unsigned int i = 0; i < loc_info_count; ++i) {
					if (!ov.get(ov.location_infos[i].line) || !ov.get(ov.location_infos[i].offset_begin) || !ov.get(ov.location_infos[i].offset_end))
						return false;
				}
			}

			unsigned int count1, count2, max_stack_yield_size,max_temporary_variables;

			if (!ov.get(count1) || !ov.get(count2) || !ov.get(max_stack_yield_size) || !ov.get(max_temporary_variables))
				return false;

			{
				unsigned int count;
				if (!ov.get(count) || count * sizeof(unsigned int) > opcodes.size())
					return false;

				ov.strings.resize(count);
				for (unsigned int i = 0; i < count; ++i) {
					owca_internal_string *ident = ov.check_string();
					ov.strings[i] = ident;
					if (ident == NULL)
						return false;
				}

				ov.indexes_1.resize(count1);
				ov.indexes_2.resize(count2);
				for (unsigned int i = 0; i < count1; ++i) {
					unsigned int index;
					if (!ov.get(index))
						return false;
					ov.indexes_1[i] = index;
				}
				for (unsigned int i = 0; i < count2; ++i) {
					unsigned int index;
					if (!ov.get(index))
						return false;
					ov.indexes_2[i] = index;
				}
			}


			ov.push_stack_size(count1 + count2, max_temporary_variables, max_stack_yield_size);
			unsigned int opcode_start = ov.offset;
			ov.begin_code_ofset = opcode_start;
			if (ov.validate_flow()) {
				unsigned int opcode_end = ov.offset;
				if (!ov.pop_stack_size())
					return false;
				if (!ov.validate_end_of_data())
					return false;
#ifdef RCDEBUG_EXECUTION
				for (auto it : ov.debugcodemap) {
					char buf[1024];
					sprintf(buf, "%5d     %s\n", it.first - 1, it.second.c_str());
					debugprint(buf);
				}
#endif
				if (result != NULL) {
					result->strings = std::move(ov.strings);
					result->top_level_max_temporary_variables = max_temporary_variables;
					result->top_level_stack_data_size = max_stack_yield_size;
					result->opcodes.resize(opcode_end - opcode_start);
					result->required_variable_name_index = std::move(ov.indexes_1);
					result->not_required_variable_name_index = std::move(ov.indexes_2);
					if (!result->opcodes.empty())
						memcpy(&result->opcodes[0], &ov.data[opcode_start], result->opcodes.size());
					result->location_infos = std::move(ov.location_infos);
				}
				return true;
			}
			return false;
		}

#ifdef RCDEBUG_EXECUTION
		void opcode_validator::setdebugcodemap(unsigned int index1, unsigned int index2, const char *txt, ...)
		{
			va_list vl;
			va_start(vl, txt);
			char buf[2048];
			vsprintf(buf, txt, vl);
			va_end(vl);
			//char buf2[2048];
			//sprintf(buf2,"%5d     %s",index2,buf);
			debugcodemap[index1] = buf;
		}
#endif
	}
}


