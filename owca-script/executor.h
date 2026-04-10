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

        public:
            Executor(VM *vm) : vm(vm) {}

            void prepare_exec(RuntimeFunctions* runtime_functions, unsigned int index, std::optional<OwcaValue> self_value, std::span<OwcaValue> arguments);
			// only used for executing main block func
			void prepare_exec(RuntimeFunctions* runtime_functions, unsigned int index, std::optional<OwcaMap> arguments);
			void prepare_exec(OwcaIterator oi);
			void prepare_exec(const OwcaCode &);

			OwcaValue run(OwcaMap *dict_output = nullptr);
        };
	}
}

#endif
