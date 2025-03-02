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
			[&](OwcaInt) -> Internal::AllocationBase * { return functions->vm->allocate<Internal::BoundFunctionSelfObject>(0, self); },
			[&](OwcaFloat) -> Internal::AllocationBase * { return functions->vm->allocate<Internal::BoundFunctionSelfObject>(0, self); },
			[&](OwcaBool) -> Internal::AllocationBase * { return functions->vm->allocate<Internal::BoundFunctionSelfObject>(0, self); },
			[&](OwcaRange) -> Internal::AllocationBase * { return functions->vm->allocate<Internal::BoundFunctionSelfObject>(0, self); },
			[&](auto v) -> Internal::AllocationBase * {
				return v.internal_value();
			}
		);
		return OwcaFunctions{ internal_value(), s };
	}

	OwcaValue OwcaFunctions::self() const
	{
		if (!self_object) return {};
		switch(self_object->kind) {
		case Internal::AllocationBase::Kind::User: return OwcaObject{ static_cast<Internal::Object*>(self_object) };
		case Internal::AllocationBase::Kind::String: return OwcaString{ static_cast<Internal::String*>(self_object) };
		case Internal::AllocationBase::Kind::RuntimeFunction: assert(false); break;
		case Internal::AllocationBase::Kind::RuntimeFunctions: return OwcaFunctions{ static_cast<Internal::RuntimeFunctions*>(self_object) };
		case Internal::AllocationBase::Kind::Map: return OwcaMap{ static_cast<Internal::DictionaryShared*>(self_object) };
		case Internal::AllocationBase::Kind::Class: return OwcaClass{ static_cast<Internal::Class*>(self_object) };
		case Internal::AllocationBase::Kind::Tuple: return OwcaTuple{ static_cast<Internal::Tuple*>(self_object) };
		case Internal::AllocationBase::Kind::Array: return OwcaArray{ static_cast<Internal::Array*>(self_object) };
		case Internal::AllocationBase::Kind::Set: return OwcaSet{ static_cast<Internal::DictionaryShared*>(self_object) };
		case Internal::AllocationBase::Kind::Iterator: return OwcaIterator{ static_cast<Internal::Iterator*>(self_object) };
		case Internal::AllocationBase::Kind::BoundSelfObject: return static_cast<Internal::BoundFunctionSelfObject*>(self_object)->self;
		}
		assert(false);
		return {};
	}
}