#include "stdafx.h"
#include "owca_functions.h"
#include "owca_vm.h"
#include "vm.h"
#include "runtime_function.h"
#include "object.h"
#include "string.h"
#include "dictionary.h"
#include "array.h"
#include "tuple.h"
#include "iterator.h"
#include "range.h"
#include "namespace.h"
#include "owca_value.h"

namespace OwcaScript {
    static Internal::BoundFunctionSelfObject bound_function_self_object_empty{ OwcaEmpty{} };
    static Internal::BoundFunctionSelfObject bound_function_self_object_completed{ OwcaCompleted{} };
    static Internal::BoundFunctionSelfObject bound_function_self_object_true{ true };
    static Internal::BoundFunctionSelfObject bound_function_self_object_false{ false };

	OwcaFunctions::OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::AllocationBase* self_object) : functions(functions), self_object(self_object) {
        if (!this->self_object) this->self_object = &bound_function_self_object_empty;
    }
	
	std::string_view OwcaFunctions::name() const
	{
		return functions->name;
	}

	OwcaValue OwcaFunctions::bind(OwcaValue self) const
	{
		auto s = self.visit(
			[](OwcaEmpty) -> Internal::AllocationBase * { return &bound_function_self_object_empty; },
			[](OwcaCompleted) -> Internal::AllocationBase * { return &bound_function_self_object_completed; },
			[](Number n) -> Internal::AllocationBase * { return Internal::current_vm().allocate<Internal::BoundFunctionSelfObject>(0, n); },
			[](bool b) -> Internal::AllocationBase * { return b ? &bound_function_self_object_true : &bound_function_self_object_false; },
			[](OwcaRange b) -> Internal::AllocationBase * { return b.internal_object(); },
			[](OwcaException oe) -> Internal::AllocationBase * { return oe.internal_owner(); },
			[](OwcaFunctions oe) -> Internal::AllocationBase * {
				if (oe.internal_self_object()) {
					return Internal::current_vm().allocate<Internal::BoundFunctionSelfObject>(0, oe);
				}
				return oe.internal_value();
			},
			[](auto v) -> Internal::AllocationBase * {
				return v.internal_value();
			}
		);
		return OwcaFunctions{ internal_value(), s };
	}

	OwcaValue OwcaFunctions::self() const
	{
        return self_object->bound_function_self_object(); 
	}

	void gc_mark_value(GenerationGC gc, const OwcaFunctions &f) {
		gc_mark_value(gc, f.functions);
		if (f.self_object)
			gc_mark_value(gc, f.self_object);
	}
}