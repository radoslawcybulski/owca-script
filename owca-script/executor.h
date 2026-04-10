#ifndef RC_OWCA_SCRIPT_EXECUTOR_H
#define RC_OWCA_SCRIPT_EXECUTOR_H

#include "stdafx.h"
#include "owca_value.h"
#include "execution_frame.h"

namespace OwcaScript {
	class OwcaCode;
	class OwcaVariable;

	namespace Internal {
        class RuntimeFunctions;
        class VM;

        class Executor {
            VM *vm;
			const size_t stack_top_level_index = 0;

			OwcaValue &push_value(OwcaValue value);
			OwcaValue pop_value();

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
