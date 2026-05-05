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

namespace OwcaScript {
	OwcaFunctions::OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::AllocationBase* self_object) : functions(functions), self_object(self_object) {}
	
	std::string_view OwcaFunctions::name() const
	{
		return functions->name;
	}

	OwcaValue OwcaFunctions::bind(OwcaValue self) const
	{
		auto s = self.visit(
			[](OwcaEmpty) -> Internal::AllocationBase * { return nullptr; },
			[&](OwcaCompleted) -> Internal::AllocationBase * { return functions->vm->allocate<Internal::BoundFunctionSelfObject>(0, self); },
			[&](Number) -> Internal::AllocationBase * { return functions->vm->allocate<Internal::BoundFunctionSelfObject>(0, self); },
			[&](bool) -> Internal::AllocationBase * { return functions->vm->allocate<Internal::BoundFunctionSelfObject>(0, self); },
			[&](OwcaRange) -> Internal::AllocationBase * { return functions->vm->allocate<Internal::BoundFunctionSelfObject>(0, self); },
			[&](OwcaException oe) -> Internal::AllocationBase * { return oe.internal_owner(); },
			[&](auto v) -> Internal::AllocationBase * {
				return v.internal_value();
			}
		);
		return OwcaFunctions{ internal_value(), s };
	}

	std::optional<OwcaValue> OwcaFunctions::self() const
	{
		if (!self_object) return std::nullopt;
		switch(self_object->kind) {
		case Internal::AllocationBase::Kind::User: return OwcaObject{ static_cast<Internal::Object*>(self_object) };
		case Internal::AllocationBase::Kind::String: return OwcaString{ static_cast<Internal::String*>(self_object) };
		case Internal::AllocationBase::Kind::RuntimeFunction: assert(false); break;
		case Internal::AllocationBase::Kind::RuntimeFunctions: return OwcaFunctions{ static_cast<Internal::RuntimeFunctions*>(self_object) };
		case Internal::AllocationBase::Kind::Map: return OwcaMap{ static_cast<Internal::DictionaryShared*>(self_object) };
		case Internal::AllocationBase::Kind::Class: return OwcaClass{ static_cast<Internal::Class*>(self_object) };
		case Internal::AllocationBase::Kind::Tuple: return OwcaTuple{ static_cast<Internal::Tuple*>(self_object) };
		case Internal::AllocationBase::Kind::Array: return OwcaArray{ static_cast<Internal::Array*>(self_object) };
		case Internal::AllocationBase::Kind::Set: return OwcaSet{ static_cast<Internal::SetShared*>(self_object) };
		case Internal::AllocationBase::Kind::Iterator: return OwcaIterator{ static_cast<Internal::Iterator*>(self_object) };
		case Internal::AllocationBase::Kind::BoundSelfObject: return static_cast<Internal::BoundFunctionSelfObject*>(self_object)->self;
		case Internal::AllocationBase::Kind::Range: return OwcaRange{ static_cast<Internal::Range*>(self_object) };
		case Internal::AllocationBase::Kind::Namespace: return OwcaNamespace{ static_cast<Internal::Namespace*>(self_object) };
		}
		assert(false);
		return {};
	}

	void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaFunctions &f) {
		gc_mark_value(vm, gc, f.functions);
		if (f.self_object)
			gc_mark_value(vm, gc, f.self_object);
	}
}