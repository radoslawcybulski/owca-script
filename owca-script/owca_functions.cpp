#include "stdafx.h"
#include "owca_functions.h"
#include "owca_vm.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript {
	OwcaFunctions::OwcaFunctions(OwcaVM& vm)
	{
		functions = Internal::VM::get(vm).allocate<Internal::RuntimeFunctions>();
	}

	void OwcaFunctions::add(Internal::RuntimeFunction rf)
	{
		auto pc = rf.param_count;
		functions->functions.insert_or_assign(pc, std::move(rf));
	}

	std::string_view OwcaFunctions::name() const
	{
		return functions->name;
	}
}