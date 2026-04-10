#ifndef RC_OWCA_SCRIPT_VM_H
#define RC_OWCA_SCRIPT_VM_H

#include "stdafx.h"
#include "ast_expr_compare.h"
#include "allocation_base.h"
#include "owca_value.h"
#include "execution_frame.h"
#include "impl_base.h"
#include "owca_variable.h"
#include "owca_code.h"

namespace OwcaScript {
	class OwcaValue;
	class OwcaCode;
	class OwcaVariable;

	namespace Internal {
		struct Object;
		struct Class;
		struct Array;
		struct RuntimeFunction;
		struct Exception;
		class Executor;

		enum class CompareKind : std::uint8_t;

		class VM {
			friend class Executor;
			friend class ExecutionFrame;
			
			AllocationEmpty root_allocated_memory;
			std::vector<ExecutionFrame> stacktrace;
			std::unordered_map<std::string, OwcaValue> builtin_objects;
			size_t current_stack_trace_index = 0;
			
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
			String *empty_string = nullptr;
			unsigned int generation_gc = 0;

			std::optional<OwcaException> exception_being_handled;
			std::optional<OwcaValue> value_to_yield;
			std::optional<ClassToken> currently_building_class;
			std::list<OwcaValue> temp_gc_protect_list;

			void initialize_builtins();

			ExecutionFrame &currently_executing_frame();
			ExecutionFrame &just_executed_executing_frame();
			ExecutionFrame &push_new_frame();

			struct BuiltinProvider;
		public:
			std::string_view current_class_in_progress;

			VM();
			~VM();
			
			class TempGCProtect {
				VM &vm;
				std::list<OwcaValue>::iterator it;
			public:
				TempGCProtect(VM &vm, OwcaValue val) : vm(vm) {
					it = vm.temp_gc_protect_list.insert(vm.temp_gc_protect_list.end(), val);
				}
				~TempGCProtect() {
					vm.temp_gc_protect_list.erase(it);
				}
			};

			void initialize_exception_object(Exception &);
			[[noreturn]] void throw_exception(Class *exc, std::string_view msg);

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
			[[noreturn]] void throw_cant_convert_to_float_message(std::string_view msg);
			[[noreturn]] void throw_cant_convert_to_integer(Number val);
			[[noreturn]] void throw_cant_convert_to_integer(std::string_view type);
			[[noreturn]] void throw_not_a_number(std::string_view type);
			[[noreturn]] void throw_overflow(std::string_view msg);
			[[noreturn]] void range_step_is_zero();

			// invalid operation exception
			[[noreturn]] void throw_cant_compare(CompareKind kind, std::string_view left, std::string_view right);
			[[noreturn]] void range_step_must_be_one_in_left_side_of_write_assign();
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

			const auto &get_builtin_objects() const { return builtin_objects; }
			OwcaValue execute_code_block(const OwcaCode&, std::optional<OwcaMap> values, OwcaMap *output_dict);
			OwcaValue execute_call(OwcaValue func, std::span<OwcaValue> arguments);
			OwcaValue resume_generator(OwcaIterator oi);
			OwcaArray create_array(std::deque<OwcaValue> arguments);
			OwcaTuple create_tuple(std::vector<OwcaValue> arguments);
			OwcaTuple create_tuple(std::pair<OwcaValue, OwcaValue> arguments);
			OwcaException create_exception();
			OwcaRange create_range();
			OwcaMap create_map(const std::span<OwcaValue> &arguments = {});
			OwcaMap create_map(const std::span<std::pair<OwcaValue, OwcaValue>> &values);
			OwcaMap create_map(const std::span<std::pair<std::string, OwcaValue>> &values);
			OwcaSet create_set(const std::span<OwcaValue> &arguments);
			OwcaString create_string_from_view(std::string_view txt);
			OwcaString create_string(OwcaValue str, size_t start, size_t end);
			OwcaString create_string(OwcaValue str, size_t count);
			OwcaString create_string(OwcaValue left, OwcaValue right);
			OwcaValue allocate_user_class(Class *cls, std::span<OwcaValue> arguments);
			OwcaValue get_identifier(unsigned int index);
			Generator iterate_value(OwcaValue val);
			std::pair<OwcaValue, OwcaValue> unpack_two_elements_or_raise(OwcaValue val);
			OwcaValue member(OwcaValue val, std::string_view key);
			std::optional<OwcaValue> try_member(OwcaValue val, std::string_view key);
			void member(OwcaValue val, std::string_view key, OwcaValue);
			Exception *is_exception(OwcaObject obj) const;

			bool compare_values(CompareKind kind, OwcaValue left, OwcaValue right);
			size_t calculate_hash(OwcaValue);
			bool calculate_if_true(OwcaValue);
			OwcaIterator create_iterator(OwcaValue );

			struct CurrentlyBuildingClassGuard {
				VM &vm;
				std::optional<ClassToken> previous;

				CurrentlyBuildingClassGuard(VM &vm, std::optional<ClassToken> token) : vm(vm) {
					previous = vm.currently_building_class;
					vm.currently_building_class = token;
				}
				~CurrentlyBuildingClassGuard() {
					vm.currently_building_class = previous;
				}
			};
			CurrentlyBuildingClassGuard set_currently_building_class(std::optional<ClassToken> token) { return CurrentlyBuildingClassGuard{ *this, token }; }
			auto get_currently_building_class() const { return currently_building_class; }

			void set_identifier(unsigned int index, OwcaValue value, bool function_write=false);
			OwcaCode currently_running_code() const;
			void run_gc();

			template <typename T, typename ... ARGS> T* allocate(size_t oversize, ARGS && ... args) {
				auto align = std::max(alignof(T), size_t(16));
				auto s = (sizeof(T) + oversize + align - 1) & ~(align - 1);
				auto p = std::aligned_alloc(align, s);
				auto p2 = new (p) T{ std::forward<ARGS>(args)... };
				p2->prev = &root_allocated_memory;
				p2->next = root_allocated_memory.next;
				p2->prev->next = p2->next->prev = p2;
				p2->vm = this;
				p2->kind = T::object_kind;
				return p2;
			}

			static VM& get(const OwcaVM &v);
		};

		void gc_mark_value(OwcaVM vm, GenerationGC ggc, const AllocationBase* ptr);
		template <std::integral T> void gc_mark_value(OwcaVM vm, GenerationGC ggc, T) {}
		template <std::floating_point T> void gc_mark_value(OwcaVM vm, GenerationGC ggc, T) {}
		inline void gc_mark_value(OwcaVM vm, GenerationGC ggc, const std::string &) {}
		inline void gc_mark_value(OwcaVM vm, GenerationGC ggc, std::string_view) {}
		template <typename T> void gc_mark_value(OwcaVM vm, GenerationGC ggc, const std::vector<T> &vct) {
			for(auto &q : vct) {
				gc_mark_value(vm, ggc, q);
			}
		}
		template <typename T> void gc_mark_value(OwcaVM vm, GenerationGC ggc, const std::deque<T> &vct) {
			for(auto &q : vct) {
				gc_mark_value(vm, ggc, q);
			}
		}
		template <typename T> void gc_mark_value(OwcaVM vm, GenerationGC ggc, const std::list<T> &vct) {
			for(auto &q : vct) {
				gc_mark_value(vm, ggc, q);
			}
		}
		template <typename K, typename V, typename ... ARGS> void gc_mark_value(OwcaVM vm, GenerationGC ggc, const std::unordered_map<K, V, ARGS...> &vct) {
			for(auto &q : vct) {
				gc_mark_value(vm, ggc, q.first);
				gc_mark_value(vm, ggc, q.second);
			}
		}

		//std::unordered_map<std::string, OwcaValue>
	}
}

#endif
