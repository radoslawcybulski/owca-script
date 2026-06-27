#ifndef RC_OWCA_SCRIPT_EXECUTOR_H
#define RC_OWCA_SCRIPT_EXECUTOR_H

#include "owca-script/owca_namespace.h"
#include "owca_exception.h"
#include "stdafx.h"
#include "owca_value.h"
#include "exec_buffer.h"
#include <optional>
#include <stdexcept>
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
		class Executor;
		class CodePosition;

		struct Operators2 {
			OwcaValue (*add)(OwcaValue left, OwcaValue right);
			OwcaValue (*sub)(OwcaValue left, OwcaValue right);
			OwcaValue (*mul)(OwcaValue left, OwcaValue right);
			OwcaValue (*div)(OwcaValue left, OwcaValue right);
			OwcaValue (*mod)(OwcaValue left, OwcaValue right);
			OwcaValue (*bin_and)(OwcaValue left, OwcaValue right);
			OwcaValue (*bin_or)(OwcaValue left, OwcaValue right);
			OwcaValue (*bin_xor)(OwcaValue left, OwcaValue right);
			OwcaValue (*bin_lshift)(OwcaValue left, OwcaValue right);
			OwcaValue (*bin_rshift)(OwcaValue left, OwcaValue right);
			bool (*less)(OwcaValue left, OwcaValue right);
			bool (*eq)(OwcaValue left, OwcaValue right);
			bool (*is)(OwcaValue left, OwcaValue right);

			Operators2();
		};
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

				friend void gc_mark_value(GenerationGC generation_gc, const ClassState &e);
			};
			struct ForState {
				static constexpr const std::uint8_t Kind = 1;
				OwcaIterator iterator;
				CodePosition continue_position = CodePosition{}, end_position = CodePosition{};
				std::uint8_t loop_control_depth = 0;

				ForState(OwcaIterator iterator) : iterator(iterator) {}

				friend void gc_mark_value(GenerationGC generation_gc, const ForState &e);
			};
			struct WhileState {
				static constexpr const std::uint8_t Kind = 2;
				CodePosition end_position = CodePosition{}, continue_position = CodePosition{};
				std::uint8_t loop_control_depth = 0;

				friend void gc_mark_value(GenerationGC generation_gc, const WhileState &e);
			};
			struct TryState {
				static constexpr const std::uint8_t Kind = 3;
				CodePosition begin_position = CodePosition{}, end_position = CodePosition{};
				CodePosition catches_pos = CodePosition{};
				TemporariesPtr temporary_ptr;
				std::optional<OwcaException> original_exception_being_handled;

				TryState(TemporariesPtr temporary_ptr) : temporary_ptr(temporary_ptr) {}

				friend void gc_mark_value(GenerationGC generation_gc, const TryState &e);
			};
			struct CatchState {
				static constexpr const std::uint8_t Kind = 4;
				std::optional<OwcaException> exception_being_handled;
				std::optional<OwcaException> original_exception_being_handled;

				friend void gc_mark_value(GenerationGC generation_gc, const CatchState &e);
			};
			struct WithState {
				static constexpr const std::uint8_t Kind = 5;
				OwcaValue context;
				bool entered = false;

				friend void gc_mark_value(GenerationGC generation_gc, const WithState &e);
			};
			using StatesType = std::variant<ClassState, ForState, WhileState, TryState, CatchState, WithState>;
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
			};
			friend void gc_mark_value(GenerationGC generation_gc, const StatesType &e);

			void update_current_top_ptrs(TemporariesPtr temporary_ptr) {
				temporary_ptr_current_top = temporary_ptr;
			}

		private:
			struct Frame {
				RuntimeFunction* runtime_function = nullptr;
				CodePosition code_position = CodePosition{};
				std::vector<StatesType> states;

				Frame() {
					states.reserve(8);
				}
				void init(RuntimeFunction* runtime_function, CodePosition code_position) {
					this->runtime_function = runtime_function;
					this->code_position = code_position;
					assert(states.empty());
					states.clear();
				}
			};
			std::vector<Frame> stacktrace_vector;
			Frame *stacktrace_current;
			std::vector<OwcaValue> values_vector;
			std::unordered_map<std::string_view, OwcaNamespace> namespaces;
			std::optional<OwcaException> exception_being_thrown;
			std::optional<OwcaException> exception_being_handled;
			TemporariesPtr temporary_ptr_current_top;
			
		public:
			struct TopPtrsKeeper {
				Executor &e;
				TemporariesPtr temporary_ptr_current_top;

				TopPtrsKeeper(Executor &e) : e(e), temporary_ptr_current_top(e.temporary_ptr_current_top) {}
				~TopPtrsKeeper() {
					e.temporary_ptr_current_top = temporary_ptr_current_top;
					e.exception_being_thrown.reset();
					e.exception_being_handled.reset();
				}
			};

			struct StackTraceState {
				Executor &e;

				StackTraceState(Executor &e, RuntimeFunction* runtime_function, CodePosition code_position) : e(e) {
					++e.stacktrace_current;
					if (e.stacktrace_current < e.stacktrace_vector.data() + e.stacktrace_vector.size()) [[likely]] {
						e.stacktrace_current->init(runtime_function, code_position);
					}
					else {
						--e.stacktrace_current;
						throw std::runtime_error("Stack overflow: too many nested function calls");
					}
				}
				~StackTraceState() {
					--e.stacktrace_current;
				}
			};
		private:
			std::tuple<OwcaValue, TemporariesPtr, CodePosition> run_opcodes(GlobalsPtr globals_ptr, const LocalsPtr locals_ptr, TemporariesPtr temporary_ptr, CodePosition code_pos);
			OwcaValue set_identifier_function(OwcaValue target, OwcaValue value);
			OwcaValue index_read(OwcaValue self, OwcaValue key);
			OwcaValue index_write(OwcaValue self, OwcaValue key, OwcaValue value);

			OwcaValue execute_function(RuntimeFunctions* runtime_functions, TemporariesPtr temporary_ptr, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			OwcaValue execute_call_from_values(TemporariesPtr temporary_ptr, unsigned int argument_count);
			OwcaValue execute_function_call_from_values(RuntimeFunctions* runtime_functions, TemporariesPtr temporary_ptr, unsigned int arg_count);
			std::optional<OwcaValue> continue_iterator(OwcaIterator oi);
			OwcaValue allocate_user_class_from_values(TemporariesPtr temporary_ptr, unsigned int arg_count);

			OwcaValue create_function(CodePosition &code_pos, GlobalsPtr globals_ptr, LocalsPtr locals_ptr);
			void process_thrown_exception(CodePosition *pos, OwcaException exc);

			std::tuple<Number, Number, Number> parse_key(OwcaValue v, OwcaValue key, Number size);
			size_t verify_key(Number v, size_t size, OwcaValue orig_key, std::string_view name);
			std::pair<size_t, size_t> verify_key(OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name);
			void complete(WithState, TemporariesPtr temporary_ptr);
			void complete_all(TemporariesPtr temporary_ptr);
        public:
            Executor();

			void clear();

			std::span<const OwcaValue> values_vector_span() const { return std::span{ values_vector.data(), values_vector.size() }; };

			OwcaNamespace execute_code_block(OwcaCode oc);
			Generator run_script_generator(Iterator *iter_object, RuntimeFunction *function, GlobalsPtr globals_ptr, std::vector<OwcaValue> values_vec, std::vector<StatesType> states_vec, CodePosition code_pos);
			OwcaValue run_script_code(RuntimeFunctionScriptFunction *function, GlobalsPtr globals_ptr, TemporariesPtr temporary_ptr, unsigned int arg_count, bool clear_locals);
			OwcaValue allocate_user_class(Class *cls, std::span<OwcaValue> arguments);
			OwcaValue execute_call(OwcaValue func, std::span<OwcaValue> arguments);
			OwcaValue execute_call(std::span<OwcaValue> arguments) {
				assert(!arguments.empty());
				return execute_call(arguments[0], arguments.subspan(1));
			}
			bool execute_compare(OwcaValue left, OwcaValue right, CompareKind kind);
			bool execute_compare_eq(OwcaValue left, OwcaValue right);
			bool execute_compare_less(OwcaValue left, OwcaValue right);
			bool execute_compare_is(OwcaValue left, OwcaValue right);

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
			[[noreturn]] void throw_invalid_operand_for_mul_string(std::string_view type, std::string_view val);
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

			friend void gc_mark_value(GenerationGC ggc, const Executor &);
		};
	}
}

#endif
