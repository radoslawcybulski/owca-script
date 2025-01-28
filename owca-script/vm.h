#ifndef RC_OWCA_SCRIPT_VM_H
#define RC_OWCA_SCRIPT_VM_H

#include "stdafx.h"
#include "owca_int.h"
#include "owca_float.h"
#include "ast_expr_compare.h"
#include "allocation_base.h"

namespace OwcaScript {
	class OwcaValue;
	class OwcaCode;

	namespace Internal {
		struct Object;
		struct Class;
		struct RuntimeFunction;

		class VM : public std::enable_shared_from_this<VM> {
			AllocationEmpty root_allocated_memory;
			struct ExecutionFrame {
				RuntimeFunctions* runtime_functions;
				RuntimeFunction* runtime_function;
				Line line;
				std::vector<OwcaValue> values;

				ExecutionFrame(Line line) : line(line) {}
			};
			std::vector<ExecutionFrame> stacktrace;
			std::vector<AllocationBase*> allocated_objects;
			unsigned int generation_gc = 0;

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
		public:
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

			void throw_division_by_zero() const;
			void throw_mod_division_by_zero() const;
			void throw_cant_convert_to_float(std::string_view type) const;
			void throw_cant_convert_to_integer(OwcaFloatInternal val) const;
			void throw_cant_convert_to_integer(std::string_view type) const;
			void throw_not_a_number(std::string_view type) const;
			void throw_cant_compare(AstExprCompare::Kind kind, std::string_view left, std::string_view right) const;
			void throw_index_out_of_range(std::string msg) const;
			void throw_value_not_indexable(std::string_view type, std::string_view key_type="") const;
			void throw_missing_member(std::string_view type, std::string_view ident) const;
			void throw_cant_call(std::string_view msg) const;
			void throw_not_callable(std::string_view type) const;
			void throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int) const;
			void throw_wrong_type(std::string_view type, std::string_view expected) const;
			void throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right) const;
			void throw_invalid_operand_for_mul_string(std::string_view val) const;
			void throw_missing_key(std::string_view key) const;
			void throw_not_hashable(std::string_view type) const;
			void throw_value_cant_have_fields(std::string_view type) const;
			void throw_missing_native(std::string_view msg) const;

			void update_execution_line(Line);
			OwcaValue execute_code_block(const OwcaCode&, const std::unordered_map<std::string, OwcaValue>* values);
			OwcaValue execute_call(const OwcaValue &func, std::span<OwcaValue> arguments);
			OwcaValue create_array(std::vector<OwcaValue> arguments);
			OwcaValue create_map(std::vector<OwcaValue> arguments);
			OwcaValue create_set(std::vector<OwcaValue> arguments);
			OwcaValue get_identifier(unsigned int index);
			Class* ensure_is_class(const OwcaValue&);
			Object* ensure_is_object(const OwcaValue&);
			OwcaValue member(const OwcaValue &val, const std::string& key);
			void member(OwcaValue &val, const std::string& key, OwcaValue);

			bool compare_values(const OwcaValue& left, const OwcaValue& right);
			size_t calculate_hash(const OwcaValue&);

			void set_identifier(unsigned int index, OwcaValue value);
			std::shared_ptr<CodeBuffer> currently_running_code() const;
			void run_gc();
			void gc_mark(AllocationBase* ptr, GenerationGC ggc);
			void gc_mark(const OwcaValue &, GenerationGC ggc);
			void gc_mark(const std::vector<OwcaValue> &, GenerationGC ggc);

			template <typename T, typename ... ARGS> T* allocate(size_t oversize, ARGS && ... args) {
				auto p = new char[sizeof(T) + oversize];
				auto p2 = new (p) T{ std::forward<ARGS>(args)... };
				p2->prev = root_allocated_memory.prev;
				p2->next = &root_allocated_memory;
				p2->prev->next = p2->next->prev = p2;
				allocated_objects.push_back(p2);
				return p2;
			}

			static VM& get(const OwcaVM& v);
		};
	}
}

#endif
