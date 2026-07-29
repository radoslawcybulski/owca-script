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
	class OwcaVariable;

	namespace Internal {
		enum class CompareKind : std::uint8_t;
        class RuntimeFunction;
		class RuntimeFunctionScriptFunction;
		class RuntimeFunctionScriptGenerator;
        class RuntimeFunctions;
        class VM;
		class Executor;
		class CodePosition;

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
        class IdentifierPtrs {
            std::array<OwcaValue*, 4> identifier_ptrs;

        public:
            IdentifierPtrs(OwcaValue *locals, OwcaValue *constants, OwcaValue *globals, OwcaValue *parent_values) :
                identifier_ptrs({ locals, constants, globals, parent_values }) {}

            GlobalsPtr get_globals_pointer() const { return GlobalsPtr{ identifier_ptrs[2] }; }

            OwcaValue &operator [] (VariableIndex index) {
                return identifier_ptrs[(int)index.kind()][index.index()];
            };

            const OwcaValue &operator [] (VariableIndex index) const {
                return identifier_ptrs[(int)index.kind()][index.index()];
            };
        };

        // struct TemporariesPtr {
        //     OwcaValue *temporaries_ptr;

        //     explicit TemporariesPtr(OwcaValue *ptr) : temporaries_ptr(ptr) {}

        //     TemporariesPtr operator+(int offset) const {
        //         return TemporariesPtr(temporaries_ptr + offset);
        //     }
        //     TemporariesPtr operator-(int offset) const {
        //         return TemporariesPtr(temporaries_ptr - offset);
        //     }

        //     TemporariesPtr &operator++() {
        //         ++temporaries_ptr;
        //         return *this;
        //     }
        //     TemporariesPtr &operator--() {
        //         --temporaries_ptr;
        //         return *this;
        //     }
        //     TemporariesPtr operator ++ ( int ) {
        //         TemporariesPtr tmp = *this;
        //         ++temporaries_ptr;
        //         return tmp;
        //     }
        //     TemporariesPtr operator -- ( int ) {
        //         TemporariesPtr tmp = *this;
        //         --temporaries_ptr;
        //         return tmp;
        //     }
        //     bool operator == (TemporariesPtr other) const {
        //         return temporaries_ptr == other.temporaries_ptr;
        //     }
        //     bool operator < (TemporariesPtr other) const {
        //         return temporaries_ptr < other.temporaries_ptr;
        //     }
        //     OwcaValue &operator [] (std::size_t index ) {
        //         return *(temporaries_ptr - index);
        //     }
        //     std::span<OwcaValue> operator [] (std::pair<std::size_t, std::size_t> indexes) {
        //         return std::span{ temporaries_ptr - indexes.first, indexes.second };
        //     }
        //     LocalsPtr locals(size_t args) {
        //         return LocalsPtr(temporaries_ptr - args);
        //     }
        // };
        struct Operators2 {
			std::array<OwcaValue (*)(OwcaValue, OwcaValue), 10> math_opers;
			//std::array<bool (*)(OwcaValue, OwcaValue), 3> compare_opers;
			// OwcaValue (*add)(OwcaValue left, OwcaValue right);
			// OwcaValue (*sub)(OwcaValue left, OwcaValue right);
			// OwcaValue (*mul)(OwcaValue left, OwcaValue right);
			// OwcaValue (*div)(OwcaValue left, OwcaValue right);
			// OwcaValue (*mod)(OwcaValue left, OwcaValue right);
			// OwcaValue (*bin_or)(OwcaValue left, OwcaValue right);
			// OwcaValue (*bin_and)(OwcaValue left, OwcaValue right);
			// OwcaValue (*bin_xor)(OwcaValue left, OwcaValue right);
			// OwcaValue (*bin_lshift)(OwcaValue left, OwcaValue right);
			// OwcaValue (*bin_rshift)(OwcaValue left, OwcaValue right);
			bool (*less)(OwcaValue left, OwcaValue right);
			bool (*eq)(OwcaValue left, OwcaValue right);
			bool (*is)(OwcaValue left, OwcaValue right);

			Operators2();
		};
        struct Operators1 {
            OwcaValue (*call)(size_t count);
            bool (*is_true)(OwcaValue value);
        };

        class Executor {
			friend class VM;

		private:
			struct Frame {
				RuntimeFunction* runtime_function = nullptr;
				CodePosition code_position = CodePosition{};

				void init(RuntimeFunction* runtime_function, CodePosition code_position) {
					this->runtime_function = runtime_function;
					this->code_position = code_position;
				}
			};
			std::vector<Frame> stacktrace_vector;
			Frame *stacktrace_current;
			std::vector<OwcaValue> values_vector;
			std::unordered_map<std::string_view, OwcaNamespace> namespaces;
			std::optional<OwcaException> exception_being_thrown;
			std::optional<OwcaException> exception_being_handled;
			LocalsPtr current_unused_locals_ptr;

		public:
			struct TopPtrsKeeper {
				Executor &e;
				LocalsPtr current_unused_locals_ptr;

				TopPtrsKeeper(Executor &e, size_t reserve_values) : e(e), current_unused_locals_ptr(e.current_unused_locals_ptr) {
                    e.current_unused_locals_ptr = e.current_unused_locals_ptr + reserve_values;
                }
				~TopPtrsKeeper() {
					e.current_unused_locals_ptr = current_unused_locals_ptr;
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
			std::tuple<OwcaValue, CodePosition> run_opcodes(const LocalsPtr locals_ptr, CodePosition code_pos);
			OwcaValue set_identifier_function(OwcaValue target, OwcaValue value);
			OwcaValue index_read(OwcaValue self, OwcaValue key);
			OwcaValue index_write(OwcaValue self, OwcaValue key, OwcaValue value);

			OwcaValue execute_function(RuntimeFunctions* runtime_functions, LocalsPtr temporary_ptr, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			OwcaValue execute_call_from_values(unsigned int argument_count);
			OwcaValue execute_function_call_from_values(RuntimeFunctions* runtime_functions, unsigned int arg_count);
			std::optional<OwcaValue> continue_iterator(OwcaIterator oi);

			OwcaValue create_function(CodePosition &code_pos, const IdentifierPtrs &identifier_ptrs);
			void process_thrown_exception(CodePosition *pos, OwcaException exc);

			std::tuple<Number, Number, Number> parse_key(OwcaValue v, OwcaValue key, Number size);
			size_t verify_key(Number v, size_t size, OwcaValue orig_key, std::string_view name);
			std::pair<size_t, size_t> verify_key(OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name);
			// void complete(WithState, TemporariesPtr temporary_ptr);
			// void complete_all(TemporariesPtr temporary_ptr);
        public:
            Executor();
			~Executor();

			void clear();

			std::span<const OwcaValue> values_vector_span() const { return std::span{ values_vector.data(), values_vector.size() }; };

			auto get_current_unused_locals_ptr() const { return current_unused_locals_ptr; }
			OwcaNamespace execute_code_block(const OwcaCodeBuffer &oc, std::shared_ptr<NativeCodeProvider> native_code_provider);
            OwcaValue execute_function_call_from_values(unsigned int arg_count);
			OwcaValue allocate_user_class_from_values(unsigned int arg_count);
			Generator run_script_generator(Iterator *iter_object, RuntimeFunctionScriptGenerator *function, std::vector<OwcaValue> values_vec, CodePosition code_pos);
			OwcaValue run_script_code(RuntimeFunctionScriptFunction *function, LocalsPtr locals, unsigned int arg_count, bool clear_locals);
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
