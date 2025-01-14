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

			PopStack prepare_exec(RuntimeFunctions* runtime_functions, unsigned int index);
			OwcaValue execute();
		public:
			VM();
			~VM();

			void throw_division_by_zero();
			void throw_mod_division_by_zero();
			void throw_cant_convert_to_float(std::string_view type);
			void throw_cant_convert_to_integer(OwcaFloatInternal val);
			void throw_cant_convert_to_integer(std::string_view type);
			void throw_not_a_number(std::string_view type);
			void throw_cant_compare(AstExprCompare::Kind kind, std::string_view left, std::string_view right);
			void throw_index_out_of_range(std::string msg);
			void throw_value_not_indexable(std::string_view type, std::string_view key_type="");
			void throw_missing_member(std::string_view type, std::string_view ident);
			void throw_not_callable(std::string_view type);
			void throw_not_callable_wrong_number_of_params(std::string_view type, unsigned int);
			void throw_wrong_type(std::string_view type, std::string_view expected);
			void throw_unsupported_operation_2(std::string_view oper, std::string_view left, std::string_view right);
			void throw_invalid_operand_for_mul_string(std::string_view val);

			void update_execution_line(Line);
			OwcaValue execute_code_block(const OwcaCode&, const std::unordered_map<std::string, OwcaValue>& values = {});
			OwcaValue execute_call(OwcaValue func, std::vector<OwcaValue> arguments);
			OwcaValue create_array(std::vector<OwcaValue> arguments);
			OwcaValue create_map(std::vector<OwcaValue> arguments);
			OwcaValue create_set(std::vector<OwcaValue> arguments);
			OwcaValue get_identifier(unsigned int index);
			void set_identifier(unsigned int index, OwcaValue value);
			std::shared_ptr<CodeBuffer> currently_running_code() const;
			void run_gc();
			void gc_mark(const OwcaValue &, GenerationGC ggc);
			void gc_mark(const std::vector<OwcaValue> &, GenerationGC ggc);

			template <typename T> T* allocate(size_t oversize = 0) {
				auto p = new char[sizeof(T) + oversize];
				auto p2 = new (p) T{};
				p2->prev = root_allocated_memory.prev;
				p2->next = &root_allocated_memory;
				p2->prev->next = p2->next->prev = p2;
				return p2;
			}
		};
	}
}

#endif
