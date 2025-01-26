#include "stdafx.h"
#include "owca_functions.h"
#include "owca_vm.h"
#include "vm.h"
#include "runtime_function.h"
#include "object.h"

namespace OwcaScript {
	OwcaFunctions::OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::Object* self_object) : functions(functions), self_object(self_object) {}
	OwcaFunctions::OwcaFunctions(Internal::RuntimeFunctions* functions, Internal::BoundFunctionSelfObject* self_object) : functions(functions), self_object(self_object) {}

	std::string_view OwcaFunctions::name() const
	{
		return functions->name;
	}
}