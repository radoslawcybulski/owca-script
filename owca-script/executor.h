#ifndef RC_OWCA_SCRIPT_EXECUTOR_H
#define RC_OWCA_SCRIPT_EXECUTOR_H

#include "stdafx.h"
#include "owca_value.h"
#include "execution_frame.h"
#include "runtime_function.h"

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
			std::optional<OwcaException> exception_in_progress;

			void push_value(OwcaValue value);
			void pop_values(size_t count);
			OwcaValue &peek_value(size_t offset) const;
			OwcaValue &peek_value_and_make_top(size_t offset) const;
			std::span<OwcaValue> peek_values(size_t offset, size_t count) const;
			
			ExecutionFrame &currently_executing_frame();
			ExecutionFrame &just_executed_executing_frame();
			ExecutionFrame &push_new_frame();
			void pop_frame();
			bool completed() const;

			void prepare_allocate_user_class(OwcaValue &return_value, Class *cls, std::span<OwcaValue> arguments);
			void prepare_resume_generator(OwcaValue &return_value, OwcaIterator oi);
			void prepare_execute_call(OwcaValue &return_value, OwcaValue func, std::span<OwcaValue> arguments);
			ExecutionFrame &prepare_execute_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			void prepare_execute_main_function(OwcaValue &return_value, RuntimeFunctions* runtime_functions, std::optional<OwcaMap> arguments);
			void prepare_execute_code_block(OwcaValue &return_value, const OwcaCode &oc);

            // void prepare_exec(OwcaValue &return_value, RuntimeFunctions* runtime_functions, unsigned int index, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			// // only used for executing main block func
			// void prepare_exec(OwcaValue &return_value, RuntimeFunctions* runtime_functions, unsigned int index, std::optional<OwcaMap> arguments);
			// void prepare_exec(OwcaValue &return_value, OwcaIterator oi);
			// void prepare_exec(OwcaValue &return_value, const OwcaCode &);
			void run(OwcaMap *dict_output = nullptr);
			void run_impl();
			void run_impl_opcodes(ExecutionFrame &frame, RuntimeFunction::ScriptFunction& sf);
			void run_impl_opcodes_execute_compare(ExecuteBufferReader &reader, CompareKind kind);
			void process_thrown_exception(OwcaException exception);
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
			struct TagIndexRead {};
			struct TagIndexWrite {};
			template <typename Tag> void run_impl_opcodes_execute_expr_oper2(ExecuteBufferReader &reader);
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
			template <typename A, typename B, typename C> OwcaEmpty expr_oper_2(A, B, C);
        public:
            Executor(VM *vm);

			void clear();

			OwcaValue execute_code_block(const OwcaCode &oc, std::optional<OwcaMap> values, OwcaMap *dict_output);
			OwcaValue resume_generator(OwcaIterator oi);
			OwcaValue allocate_user_class(Class *cls, std::span<OwcaValue> arguments);
			OwcaValue execute_call(OwcaValue func, std::span<OwcaValue> arguments);
        };
	}
}

#endif
