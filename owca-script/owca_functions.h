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
		std::optional<OwcaValue> self() const;
		OwcaValue bind(OwcaValue self) const;
		bool is(OwcaFunctions other) const { return functions == other.functions && self_object == other.self_object; }

		bool operator == (OwcaFunctions other) const { return functions == other.functions && self_object == other.self_object; }
		bool operator != (OwcaFunctions other) const { return !(*this == other); }

		friend void gc_mark_value(const OwcaVM &vm, GenerationGC gc, const OwcaFunctions &);
	};
}

#endif
