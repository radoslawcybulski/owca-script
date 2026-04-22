#ifndef RC_OWCA_SCRIPT_EXECUTOR_H
#define RC_OWCA_SCRIPT_EXECUTOR_H

#include "stdafx.h"
#include "owca_value.h"
#include "execution_frame.h"
#include "runtime_function.h"
#include "exec_buffer.h"

namespace OwcaScript {
	class OwcaCode;
	class OwcaVariable;

	namespace Internal {
		enum class CompareKind : std::uint8_t;
        class RuntimeFunctions;
        class VM;

        class Executor {
            VM *vm;
			const size_t stack_top_level_index = 0;
	        bool exit = false;

			size_t currently_executing_frame_index() const;
			ExecutionFrame &currently_executing_frame();
			void pop_frame();
			bool completed() const;

			void prepare_allocate_user_class(OwcaValue &return_value, Class *cls, std::span<OwcaValue> arguments, bool exception_for_throwing_construction = false);
			void prepare_resume_generator(OwcaValue &return_value, OwcaIterator oi);
			void prepare_execute_call(OwcaValue &return_value, OwcaValue func, std::span<OwcaValue> arguments);
			bool prepare_execute_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			void prepare_execute_main_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments);
			void prepare_execute_code_block(OwcaValue &return_value, const OwcaCode &oc);

            // void prepare_exec(OwcaValue &return_value, RuntimeFunctions* runtime_functions, unsigned int index, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			// // only used for executing main block func
			// void prepare_exec(OwcaValue &return_value, RuntimeFunctions* runtime_functions, unsigned int index, std::optional<OwcaMap> arguments);
			// void prepare_exec(OwcaValue &return_value, OwcaIterator oi);
			// void prepare_exec(OwcaValue &return_value, const OwcaCode &);
			void run();
			[[noreturn]] void run_and_throw();
			void run_impl_opcodes(ExecutionFrame &frame, RuntimeFunction::ScriptFunction& sf);
			bool run_impl_opcodes_execute_compare(ExecutionFrame &frame, ExecuteBufferReader::StartOfCode start_code, ExecuteBufferReader::Position &pos, CompareKind kind);
			void process_thrown_exception(ExecuteBufferReader::Position *pos);
			struct TagBinOr {};
			struct TagBinAnd {};
			struct TagBinXor {};
			struct TagBinLShift {};
			struct TagBinRShift {};
			struct TagAdd {};
			struct TagSub {};
			struct TagMul {};
			struct TagDiv {};
			struct TagMod {};
			template <typename Tag> void run_impl_opcodes_execute_expr_oper2(ExecutionFrame &frame);
			Number expr_oper_2(TagAdd, Number left, Number right);
			OwcaArray expr_oper_2(TagAdd, OwcaArray left, OwcaArray right);
			OwcaTuple expr_oper_2(TagAdd, OwcaTuple left, OwcaTuple right);
			OwcaString expr_oper_2(TagAdd, OwcaString left, OwcaString right);
			Number expr_oper_2(TagSub, Number left, Number right);
			Number expr_oper_2(TagMul, Number left, Number right);
			OwcaString expr_oper_2(TagMul, OwcaString left, Number right);
			OwcaString expr_oper_2(TagMul, Number left, OwcaString right);
			OwcaArray expr_oper_2(TagMul, OwcaArray left, Number right);
			OwcaArray expr_oper_2(TagMul, Number left, OwcaArray right);
			OwcaTuple expr_oper_2(TagMul, OwcaTuple left, Number right);
			OwcaTuple expr_oper_2(TagMul, Number left, OwcaTuple right);
			Number expr_oper_2(TagDiv, Number left, Number right);
			Number expr_oper_2(TagMod, Number left, Number right);
			Number expr_oper_2(TagBinOr, Number left, Number right);
			Number expr_oper_2(TagBinAnd, Number left, Number right);
			Number expr_oper_2(TagBinXor, Number left, Number right);
			Number expr_oper_2(TagBinLShift, Number left, Number right);
			Number expr_oper_2(TagBinRShift, Number left, Number right);
			template <typename A, typename B, typename C> OwcaEmpty expr_oper_2(A, B, C);

			void prepare_throw_division_by_zero();
			void prepare_throw_mod_division_by_zero();
			void prepare_throw_cant_convert_to_float(std::string_view type);
			void prepare_throw_cant_convert_to_float_message(std::string_view msg);
			void prepare_throw_cant_convert_to_integer(Number val);
			void prepare_throw_cant_convert_to_integer(std::string_view type);
			void prepare_throw_not_a_number(std::string_view type);
			void prepare_throw_overflow(std::string_view msg);
			void prepare_throw_range_step_is_zero();
			void prepare_throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right);
			void prepare_throw_range_step_must_be_one_in_left_side_of_write_assign();
			void prepare_throw_index_out_of_range(std::string msg);
			void prepare_throw_string_too_large(size_t size);
			void prepare_throw_value_not_indexable(std::string_view type, std::string_view key_type="");
			void prepare_throw_missing_member(std::string_view type, std::string_view ident);
			void prepare_throw_cant_call(std::string_view msg);
			void prepare_throw_not_callable(std::string_view type);
			void prepare_throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int);
			void prepare_throw_wrong_type(std::string_view type, std::string_view expected);
			void prepare_throw_wrong_type(std::string_view msg);
			void prepare_throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right);
			void prepare_throw_invalid_operand_for_mul_string(std::string_view val);
			void prepare_throw_missing_key(std::string_view key);
			void prepare_throw_not_hashable(std::string_view type);
			void prepare_throw_value_cant_have_fields(std::string_view type);
			void prepare_throw_missing_native(std::string_view msg);
			void prepare_throw_not_iterable(std::string_view type);
			void prepare_throw_readonly(std::string_view msg);
			void prepare_throw_cant_return_value_from_generator();
			void prepare_throw_container_is_empty();
			void prepare_throw_not_implemented(std::string_view msg);
			void prepare_throw_dictionary_changed(bool is_dict);
			void prepare_throw_too_many_elements(size_t expected);
			void prepare_throw_not_enough_elements(size_t expected, size_t got);
			void prepare_throw_cpp_exception(std::string_view msg);

			std::optional<std::tuple<Number, Number, Number>> parse_key(VM *vm, OwcaValue v, OwcaValue key, Number size);
			std::optional<size_t> verify_key(VM *vm, Number v, size_t size, OwcaValue orig_key, std::string_view name);
			std::optional<std::pair<size_t, size_t>> verify_key(VM *vm, OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name);

        public:
            Executor(VM *vm);

			void clear();

			OwcaValue execute_code_block(const OwcaCode &oc, std::optional<OwcaMap> values, OwcaMap *dict_output);
			OwcaValue resume_generator(OwcaIterator oi);
			OwcaValue allocate_user_class(Class *cls, std::span<OwcaValue> arguments);
			OwcaValue execute_call(OwcaValue func, std::span<OwcaValue> arguments);

			[[noreturn]] void throw_division_by_zero();
			[[noreturn]] void throw_mod_division_by_zero();
			[[noreturn]] void throw_cant_convert_to_float(std::string_view type);
			[[noreturn]] void throw_cant_convert_to_float_message(std::string_view msg);
			[[noreturn]] void throw_cant_convert_to_integer(Number val);
			[[noreturn]] void throw_cant_convert_to_integer(std::string_view type);
			[[noreturn]] void throw_not_a_number(std::string_view type);
			[[noreturn]] void throw_overflow(std::string_view msg);
			[[noreturn]] void throw_range_step_is_zero();
			[[noreturn]] void throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right);
			[[noreturn]] void throw_range_step_must_be_one_in_left_side_of_write_assign();
			[[noreturn]] void throw_index_out_of_range(std::string msg);
			[[noreturn]] void throw_string_too_large(size_t size);
			[[noreturn]] void throw_value_not_indexable(std::string_view type, std::string_view key_type="");
			[[noreturn]] void throw_missing_member(std::string_view type, std::string_view ident);
			[[noreturn]] void throw_cant_call(std::string_view msg);
			[[noreturn]] void throw_not_callable(std::string_view type);
			[[noreturn]] void throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int);
			[[noreturn]] void throw_wrong_type(std::string_view type, std::string_view expected);
			[[noreturn]] void throw_wrong_type(std::string_view msg);
			[[noreturn]] void throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right);
			[[noreturn]] void throw_invalid_operand_for_mul_string(std::string_view val);
			[[noreturn]] void throw_missing_key(std::string_view key);
			[[noreturn]] void throw_not_hashable(std::string_view type);
			[[noreturn]] void throw_value_cant_have_fields(std::string_view type);
			[[noreturn]] void throw_missing_native(std::string_view msg);
			[[noreturn]] void throw_not_iterable(std::string_view type);
			[[noreturn]] void throw_readonly(std::string_view msg);
			[[noreturn]] void throw_cant_return_value_from_generator();
			[[noreturn]] void throw_container_is_empty();
			[[noreturn]] void throw_not_implemented(std::string_view msg);
			[[noreturn]] void throw_dictionary_changed(bool is_dict);
			[[noreturn]] void throw_too_many_elements(size_t expected);
			[[noreturn]] void throw_not_enough_elements(size_t expected, size_t got);
		};
	}
}

#endif
