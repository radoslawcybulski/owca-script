#include "stdafx.h"
#include "owca_functions.h"
#include "owca_vm.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript {
	OwcaFunctions::OwcaFunctions(OwcaVM& vm)
	{
		functions = vm.vm->allocate<Internal::RuntimeFunctions>();
	}

	void OwcaFunctions::add(Internal::RuntimeFunction rf)
	{
		auto pc = rf.param_count;
		functions->functions[pc] = std::move(rf);
	}
}