#ifndef RC_OWCA_SCRIPT_EXECUTOR_H
#define RC_OWCA_SCRIPT_EXECUTOR_H

#include "owca-script/owca_namespace.h"
#include "owca_exception.h"
#include "stdafx.h"
#include "owca_value.h"
#include "execution_frame.h"
#include "exec_buffer.h"
#include <optional>
#include <unordered_map>
#include <vector>

namespace OwcaScript {
	class OwcaCode;
	class OwcaVariable;

	namespace Internal {
		enum class CompareKind : std::uint8_t;
        class RuntimeFunction;
		class RuntimeFunctionScriptFunction;
        class RuntimeFunctions;
        class VM;

        class Executor {
			friend class VM;

		public:
			struct LocalsPtr {
				OwcaValue *local_values_ptr;

				explicit LocalsPtr(OwcaValue *ptr) : local_values_ptr(ptr) {}

				LocalsPtr operator+(int offset) const {
					return LocalsPtr(local_values_ptr + offset);
				}
				LocalsPtr operator-(int offset) const {
					return LocalsPtr(local_values_ptr - offset);
				}
				LocalsPtr &operator++() {
					++local_values_ptr;
					return *this;
				}
				LocalsPtr &operator--() {
					--local_values_ptr;
					return *this;
				}
				LocalsPtr operator ++ ( int ) {
					LocalsPtr tmp = *this;
					++local_values_ptr;
					return tmp;
				}
				LocalsPtr operator -- ( int ) {
					LocalsPtr tmp = *this;
					--local_values_ptr;
					return tmp;
				}
				OwcaValue &operator [] (std::size_t index) const {
					return local_values_ptr[index];
				}
			};
			struct GlobalsPtr {
				OwcaValue *global_values_ptr;

				explicit GlobalsPtr(OwcaValue *ptr) : global_values_ptr(ptr) {}

				OwcaValue &operator [] (std::size_t index) const {
					return global_values_ptr[index];
				}
			};			
			struct TemporariesPtr {
				OwcaValue *temporaries_ptr;

				explicit TemporariesPtr(OwcaValue *ptr) : temporaries_ptr(ptr) {}

				TemporariesPtr operator+(int offset) const {
					return TemporariesPtr(temporaries_ptr + offset);
				}
				TemporariesPtr operator-(int offset) const {
					return TemporariesPtr(temporaries_ptr - offset);
				}

				TemporariesPtr &operator++() {
					++temporaries_ptr;
					return *this;
				}
				TemporariesPtr &operator--() {
					--temporaries_ptr;
					return *this;
				}
				TemporariesPtr operator ++ ( int ) {
					TemporariesPtr tmp = *this;
					++temporaries_ptr;
					return tmp;
				}
				TemporariesPtr operator -- ( int ) {
					TemporariesPtr tmp = *this;
					--temporaries_ptr;
					return tmp;
				}
				bool operator == (TemporariesPtr other) const {
					return temporaries_ptr == other.temporaries_ptr;
				}
				bool operator < (TemporariesPtr other) const {
					return temporaries_ptr < other.temporaries_ptr;
				}
				OwcaValue &operator [] (std::size_t index ) {
					return *(temporaries_ptr - index);
				}
				std::span<OwcaValue> operator [] (std::pair<std::size_t, std::size_t> indexes) {
					return std::span{ temporaries_ptr - indexes.first, indexes.second };
				}
				LocalsPtr locals(size_t args) {
					return LocalsPtr(temporaries_ptr - args);
				}
			};

			struct ClassState {
				static constexpr const std::uint8_t Kind = 0;
				std::string_view name, full_name;
				Class *cls;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ClassState &e);
			};
			struct ForState {
				static constexpr const std::uint8_t Kind = 1;
				std::uint64_t index = (std::uint64_t)-1;
				OwcaIterator iterator;
				ExecuteBufferReader::Position continue_position, end_position;
				std::uint32_t loop_index = 0;
				std::uint8_t loop_control_depth = 0;

				ForState(OwcaIterator iterator) : iterator(iterator) {}

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const ForState &e);
			};
			struct WhileState {
				static constexpr const std::uint8_t Kind = 2;
				std::uint64_t index = (std::uint64_t)-1;
				ExecuteBufferReader::Position end_position, continue_position;
				std::uint32_t loop_index = 0, value_index = 0;
				std::uint8_t loop_control_depth = 0;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const WhileState &e);
			};
			struct TryState {
				static constexpr const std::uint8_t Kind = 3;
				ExecuteBufferReader::Position begin_position, end_position;
				ExecuteBufferReader::Position catches_pos;
				TemporariesPtr temporary_ptr;
				std::optional<OwcaException> original_exception_being_handled;

				TryState(TemporariesPtr temporary_ptr) : temporary_ptr(temporary_ptr) {}

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const TryState &e);
			};
			struct CatchState {
				static constexpr const std::uint8_t Kind = 4;
				std::optional<OwcaException> exception_being_handled;
				std::optional<OwcaException> original_exception_being_handled;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const CatchState &e);
			};
			struct WithState {
				static constexpr const std::uint8_t Kind = 5;
				OwcaValue context;
				bool entered = false;

				friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const WithState &e);
			};
			struct EmptyState {
				static constexpr const std::uint8_t Kind = 255;
			};
			using StatesType = std::variant<EmptyState, ClassState, ForState, WhileState, TryState, CatchState, WithState>;
			struct StatesTypePtr {
				StatesType *states_type_ptr;

				explicit StatesTypePtr(StatesType *ptr) : states_type_ptr(ptr) {}

				StatesTypePtr operator+(int offset) const {
					return StatesTypePtr(states_type_ptr + offset);
				}
				StatesTypePtr operator-(int offset) const {
					return StatesTypePtr(states_type_ptr - offset);
				}
				StatesTypePtr &operator++() {
					++states_type_ptr;
					return *this;
				}
				StatesTypePtr &operator--() {					--states_type_ptr;
					return *this;
				}
				StatesTypePtr operator ++ ( int ) {
					StatesTypePtr tmp = *this;
					++states_type_ptr;
					return tmp;
				}
				StatesTypePtr operator -- ( int ) {
					StatesTypePtr tmp = *this;
					--states_type_ptr;
					return tmp;
				}
				bool empty() const {
					return std::get_if<EmptyState>(states_type_ptr - 1) != nullptr;
				}
			};
			friend void gc_mark_value(OwcaVM vm, GenerationGC generation_gc, const StatesType &e);

			void update_current_top_ptrs(TemporariesPtr temporary_ptr, StatesTypePtr states_ptr) {
				temporary_ptr_current_top = temporary_ptr;
				states_ptr_current_top = states_ptr;
			}

		private:
            VM *vm;
			struct Frame {
				RuntimeFunction* runtime_function = nullptr;
				ExecuteBufferReader::Position code_position{ 0 };
			};
			std::vector<Frame> stacktrace;
			std::vector<OwcaValue> values_vector;
			std::vector<StatesType> states_vector;
			std::unordered_map<std::string_view, OwcaNamespace> namespaces;
			std::optional<OwcaException> exception_being_thrown;
			std::optional<OwcaException> exception_being_handled;
			TemporariesPtr temporary_ptr_current_top;
			StatesTypePtr states_ptr_current_top;
			
		public:
			struct TopPtrsKeeper {
				Executor &e;
				TemporariesPtr temporary_ptr_current_top;
				StatesTypePtr states_ptr_current_top;

				TopPtrsKeeper(Executor &e) : e(e), temporary_ptr_current_top(e.temporary_ptr_current_top), states_ptr_current_top(e.states_ptr_current_top) {}
				~TopPtrsKeeper() {
					e.temporary_ptr_current_top = temporary_ptr_current_top;
					e.states_ptr_current_top = states_ptr_current_top;
					e.exception_being_thrown.reset();
					e.exception_being_handled.reset();
				}
			};

			struct StackTraceState {
				Executor &e;

				StackTraceState(Executor &e, RuntimeFunction* runtime_function, ExecuteBufferReader::Position code_position) : e(e) {
					e.stacktrace.push_back({ runtime_function, code_position });
				}
				~StackTraceState() {
					e.stacktrace.pop_back();
				}
			};
		private:
			struct Interface {
				OwcaValue (*call)(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr, unsigned int arg_count);
			};
			static std::array<Interface, static_cast<size_t>(OwcaValueKind::_Count)> interfaces;
			static std::array<Interface, static_cast<size_t>(OwcaValueKind::_Count)> construct_interfaces();

            static OwcaValue exec_call_cant(Executor &vm, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr, unsigned int arg_count);
            static OwcaValue exec_call_function(Executor &vm, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr, unsigned int arg_count);
			static OwcaValue exec_call_class(Executor &vm, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr, unsigned int arg_count);

			// void prepare_allocate_user_class(OwcaValue &return_value, Class *cls, std::span<OwcaValue> arguments, bool exception_for_throwing_construction = false);
			// void prepare_resume_generator(OwcaValue &return_value, OwcaIterator oi);
			// void prepare_execute_call(OwcaValue &return_value, OwcaValue func, std::span<OwcaValue> arguments);
			// bool prepare_execute_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			// void prepare_execute_main_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments);
			// void prepare_execute_code_block(OwcaValue &return_value, const OwcaCode &oc);

            // void prepare_exec(OwcaValue &return_value, RuntimeFunctions* runtime_functions, unsigned int index, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			// // only used for executing main block func
			// void prepare_exec(OwcaValue &return_value, RuntimeFunctions* runtime_functions, unsigned int index, std::optional<OwcaMap> arguments);
			// void prepare_exec(OwcaValue &return_value, OwcaIterator oi);
			// void prepare_exec(OwcaValue &return_value, const OwcaCode &);
			std::tuple<OwcaValue, TemporariesPtr, StatesTypePtr, ExecuteBufferReader::Position> run_opcodes(GlobalsPtr globals_ptr, const LocalsPtr locals_ptr, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, StartOfCode start_code, ExecuteBufferReader::Position code_pos);
			//Generator run_native_generator(Iterator *iter_object, RuntimeFunction *function, RuntimeFunction::NativeGenerator& ng, std::span<OwcaValue> arguments);
			OwcaValue set_identifier_function(OwcaValue target, OwcaValue value);
			OwcaValue index_read(OwcaValue self, OwcaValue key);
			OwcaValue index_write(OwcaValue self, OwcaValue key, OwcaValue value);

			// OwcaValue run_script_function(RuntimeFunction *function, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, RuntimeFunction::ScriptFunction& sf, bool clear_locals = true);
			//OwcaValue run_native_function(RuntimeFunction *function, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, RuntimeFunction::NativeFunction& sf);
			//OwcaValue start_native_generator(RuntimeFunction *function, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, RuntimeFunction::NativeGenerator& ng);
			//OwcaValue start_script_generator(RuntimeFunction *function, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, RuntimeFunction::ScriptFunction& sf);
			OwcaValue execute_function(RuntimeFunctions* runtime_functions, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			OwcaValue execute_call_from_values(TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int argument_count);
			OwcaValue execute_function_call_from_values(RuntimeFunctions* runtime_functions, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, bool has_self, unsigned int arg_count);
			std::optional<OwcaValue> continue_iterator(OwcaIterator oi);
			OwcaValue allocate_user_class_from_values(TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count);

			// void run_impl_opcodes(ExecutionFrame &frame, RuntimeFunction::ScriptFunction& sf);
			ExecuteBufferReader::Position run_impl_opcodes_execute_compare(TemporariesPtr temporary_ptr, StartOfCode start_code, ExecuteBufferReader::Position pos, CompareKind kind, const std::unordered_map<const unsigned char *, Internal::DataKind> &data_kinds);
			OwcaValue create_function(StartOfCode start_code, ExecuteBufferReader::Position &code_pos, GlobalsPtr globals_ptr, LocalsPtr locals_ptr, StatesTypePtr states_ptr, const std::unordered_map<const unsigned char *, Internal::DataKind> &data_kinds);
			// false if current frame can't handle the exception, true otherwise
			void process_thrown_exception(ExecuteBufferReader::Position *pos, StatesTypePtr &states_ptr, OwcaException exc);

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
			template <typename Tag> void run_impl_opcodes_execute_expr_oper2(TemporariesPtr &temporary_ptr);
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

			// void prepare_throw_division_by_zero();
			// void prepare_throw_mod_division_by_zero();
			// void prepare_throw_cant_convert_to_float(std::string_view type);
			// void prepare_throw_cant_convert_to_float_message(std::string_view msg);
			// void prepare_throw_cant_convert_to_integer(Number val);
			// void prepare_throw_cant_convert_to_integer(std::string_view type);
			// void prepare_throw_not_a_number(std::string_view type);
			// void prepare_throw_overflow(std::string_view msg);
			// void prepare_throw_range_step_is_zero();
			// void prepare_throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right);
			// void prepare_throw_range_step_must_be_one_in_left_side_of_write_assign();
			// void prepare_throw_index_out_of_range(std::string msg);
			// void prepare_throw_string_too_large(size_t size);
			// void prepare_throw_value_not_indexable(std::string_view type, std::string_view key_type="");
			// void prepare_throw_missing_member(std::string_view type, std::string_view ident);
			// void prepare_throw_cant_call(std::string_view msg);
			// void prepare_throw_not_callable(std::string_view type);
			// void prepare_throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int);
			// void prepare_throw_wrong_type(std::string_view type, std::string_view expected);
			// void prepare_throw_wrong_type(std::string_view msg);
			// void prepare_throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right);
			// void prepare_throw_invalid_operand_for_mul_string(std::string_view val);
			// void prepare_throw_missing_key(std::string_view key);
			// void prepare_throw_not_hashable(std::string_view type);
			// void prepare_throw_value_cant_have_fields(std::string_view type);
			// void prepare_throw_missing_native(std::string_view msg);
			// void prepare_throw_not_iterable(std::string_view type);
			// void prepare_throw_readonly(std::string_view msg);
			// void prepare_throw_cant_return_value_from_generator();
			// void prepare_throw_container_is_empty();
			// void prepare_throw_not_implemented(std::string_view msg);
			// void prepare_throw_dictionary_changed(bool is_dict);
			// void prepare_throw_too_many_elements(size_t expected);
			// void prepare_throw_not_enough_elements(size_t expected, size_t got);
			// void prepare_throw_cpp_exception(std::string_view msg);

			std::tuple<Number, Number, Number> parse_key(VM *vm, OwcaValue v, OwcaValue key, Number size);
			size_t verify_key(VM *vm, Number v, size_t size, OwcaValue orig_key, std::string_view name);
			std::pair<size_t, size_t> verify_key(VM *vm, OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name);
        public:
            Executor(VM *vm);

			void clear();

			std::span<const OwcaValue> values_vector_span() const { return std::span{ values_vector.data(), values_vector.size() }; };
			std::span<const StatesType> states_vector_span() const { return std::span{ states_vector.data(), states_vector.size() }; };

			OwcaNamespace execute_code_block(OwcaCode oc);
			Generator run_script_generator(Iterator *iter_object, RuntimeFunction *function, GlobalsPtr globals_ptr, std::vector<OwcaValue> values_vec, std::vector<StatesType> states_vec, ExecuteBufferReader::Position code_pos);
			OwcaValue run_script_code(RuntimeFunctionScriptFunction *function, GlobalsPtr globals_ptr, TemporariesPtr temporary_ptr, StatesTypePtr states_ptr, unsigned int arg_count, bool clear_locals);
			OwcaValue allocate_user_class(Class *cls, std::span<OwcaValue> arguments);
			OwcaValue execute_call(OwcaValue func, std::span<OwcaValue> arguments);
			OwcaValue execute_call(std::span<OwcaValue> arguments) {
				assert(!arguments.empty());
				return execute_call(arguments[0], arguments.subspan(1));
			}

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

			friend void gc_mark_value(OwcaVM vm, GenerationGC ggc, const Executor &);
		};
	}
}

#endif
