#ifndef RC_OWCA_SCRIPT_OWCA_FUNCTIONS_H
#define RC_OWCA_SCRIPT_OWCA_FUNCTIONS_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaVM;
	class OwcaValue;

	namespace Internal {
		struct AllocationBase;
		struct RuntimeFunctions;
	}
	class OwcaFunctions {
		Internal::RuntimeFunctions* functions = nullptr;
		Internal::AllocationBase* self_object = nullptr;
	public:
		OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::AllocationBase* self_object = nullptr);
		
		auto internal_value() const { return functions; }
		auto internal_self_object() const { return self_object; }

		std::string_view name() const;
		OwcaValue self() const;
		OwcaValue bind(OwcaValue self) const;
	};
}

#endif
