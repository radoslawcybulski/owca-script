#ifndef RC_OWCA_SCRIPT_EXECUTOR_COMPARE_H
#define RC_OWCA_SCRIPT_EXECUTOR_COMPARE_H

#include "stdafx.h"
#include "owca_value.h"

namespace OwcaScript {
	class OwcaCode;
	class OwcaVariable;

	namespace Internal {
        enum class CompareKind : std::uint8_t;

		enum class CompareResult {
			False, True, NotExecuted
		};
        CompareResult execute_compare(VM *vm, CompareKind kind, OwcaValue left, OwcaValue right);
	}
}

#endif
