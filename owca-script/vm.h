#ifndef RC_OWCA_SCRIPT_VM_H
#define RC_OWCA_SCRIPT_VM_H

#include "stdafx.h"
#include "owca_int.h"
#include "owca_float.h"
#include "ast_expr_compare.h"
#include "allocation_base.h"
#include "owca_value.h"
#include "execution_frame.h"
#include "impl_base.h"
#include "owca_variable.h"

namespace OwcaScript {
	class OwcaValue;
	class OwcaCode;
	class OwcaVariable;

	namespace Internal {
		struct Object;
		struct Class;
		struct Array;
		struct RuntimeFunction;
		struct IteratorBase;
		struct Exception;
		enum class CompareKind;

		class VM {
			AllocationEmpty root_allocated_memory;
			std::vector<ExecutionFrame> stacktrace;
			std::vector<AllocationBase*> allocated_objects;
			std::unordered_map<std::string, OwcaValue> builtin_objects;
			std::unordered_map<std::string_view, OwcaValue> small_strings;
			
			Class *c_nul = nullptr;
			Class *c_completed = nullptr;
			Class *c_range = nullptr;
			Class *c_bool = nullptr;
			Class *c_int = nullptr;
			Class *c_float = nullptr;
			Class *c_string = nullptr;
			Class *c_function = nullptr;
			Class *c_map = nullptr;
			Class *c_class = nullptr;
			Class *c_tuple = nullptr;
			Class *c_array = nullptr;
			Class *c_set = nullptr;
			Class *c_stack_frame = nullptr;
			Class *c_stack_trace = nullptr;
			Class *c_exception = nullptr;
			Class *c_math_exception = nullptr;
			Class *c_invalid_operation_exception = nullptr;
			Class *c_iterator = nullptr;
			Tuple *empty_tuple = nullptr;
			unsigned int generation_gc = 0;

			std::optional<OwcaException> exception_being_handled;
			std::optional<OwcaValue> value_to_yield;

			struct PopStack {
				VM* vm = nullptr;

				PopStack(VM* vm) : vm(vm) {}
				PopStack(PopStack&& o) : vm(o.vm) { o.vm = nullptr; }
				~PopStack() {
					if (vm)
						vm->stacktrace.pop_back();
				}
			};

			PopStack prepare_exec(RuntimeFunctions* runtime_functions, unsigned int index, bool has_self_value);
			OwcaValue execute();
			Generator execute_generator(ImplStat::State &state, ImplStat *body);
			void initialize_builtins();

			struct BuiltinProvider;
		public:
			OwcaVariableSet global_variables;

			struct AllocatedObjectsPointer {
				std::vector<AllocationBase*> &allocated_objects;
				size_t size;

				AllocatedObjectsPointer(VM &vm) : allocated_objects(vm.allocated_objects) {
					size = allocated_objects.size();
				}
				~AllocatedObjectsPointer() {
					assert(allocated_objects.size() >= size);
					allocated_objects.resize(size);
				}
			};

			VM();
			~VM();
			
			void initialize_exception_object(Exception &);
			void throw_exception(Class *exc, std::string_view msg);

			struct ExceptionHandlingSentinel {
				VM &vm;
				std::optional<OwcaException> previous;

				ExceptionHandlingSentinel(VM &vm, OwcaException oe) : vm(vm) {
					previous = vm.exception_being_handled;
					vm.exception_being_handled = oe;
				}
				~ExceptionHandlingSentinel() {
					vm.exception_being_handled = previous;
				}
			};

			// math exception
			[[noreturn]] void throw_division_by_zero();
			[[noreturn]] void throw_mod_division_by_zero();
			[[noreturn]] void throw_cant_convert_to_float(std::string_view type);
			[[noreturn]] void throw_cant_convert_to_integer(OwcaFloatInternal val);
			[[noreturn]] void throw_cant_convert_to_integer(std::string_view type);
			[[noreturn]] void throw_not_a_number(std::string_view type);
			[[noreturn]] void throw_overflow(std::string_view msg);

			// invalid operation exception
			[[noreturn]] void throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right);
			[[noreturn]] void throw_index_out_of_range(std::string msg);
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

			void update_execution_line(Line);
			const auto &get_builtin_objects() const { return builtin_objects; }
			OwcaValue execute_code_block(const OwcaCode&, OwcaValue values, OwcaValue *output_dict);
			OwcaValue execute_call(OwcaValue func, std::span<OwcaValue> arguments);
			OwcaValue resume_generator(OwcaIterator oi);
			void set_yield_value(OwcaValue v);
			OwcaValue create_array(std::deque<OwcaValue> arguments);
			OwcaValue create_tuple(std::vector<OwcaValue> arguments);
			OwcaValue create_exception();
			OwcaValue create_map(const std::vector<OwcaValue> &arguments = {});
			OwcaValue create_map(const std::vector<std::pair<OwcaValue, OwcaValue>> &values);
			OwcaValue create_map(const std::vector<std::pair<std::string, OwcaValue>> &values);
			OwcaValue create_set(const std::vector<OwcaValue> &arguments);
			OwcaValue create_string(std::string txt);
			OwcaValue create_string_from_view(std::string_view txt);
			OwcaValue create_string(OwcaValue str, size_t start, size_t end);
			OwcaValue create_string(OwcaValue str, size_t count);
			OwcaValue create_string(OwcaValue left, OwcaValue right);
			OwcaValue get_identifier(unsigned int index);
			OwcaValue member(OwcaValue val, const std::string& key);
			std::optional<OwcaValue> try_member(OwcaValue val, const std::string& key);
			void member(OwcaValue val, const std::string& key, OwcaValue);
			Exception *is_exception(OwcaObject obj) const;

			bool compare_values(CompareKind kind, OwcaValue left, OwcaValue right);
			size_t calculate_hash(OwcaValue);
			bool calculate_if_true(OwcaValue);
			OwcaIterator create_iterator(OwcaValue );

			void set_identifier(unsigned int index, OwcaValue value, bool function_write=false);
			std::shared_ptr<CodeBuffer> currently_running_code() const;
			void run_gc();
			void gc_mark(AllocationBase* ptr, GenerationGC ggc);
			void gc_mark(OwcaValue , GenerationGC ggc);
			void gc_mark(const std::vector<OwcaValue> &, GenerationGC ggc);
			void gc_mark(const std::deque<OwcaValue> &, GenerationGC ggc);

			template <typename T, typename ... ARGS> T* allocate(size_t oversize, ARGS && ... args) {
				auto p = new char[sizeof(T) + oversize];
				auto p2 = new (p) T{ std::forward<ARGS>(args)... };
				p2->prev = &root_allocated_memory;
				p2->next = root_allocated_memory.next;
				p2->prev->next = p2->next->prev = p2;
				p2->vm = this;
				p2->kind = T::object_kind;
				allocated_objects.push_back(p2);
				return p2;
			}

			static VM& get(const OwcaVM &v);
		};
	}
}

#endif
