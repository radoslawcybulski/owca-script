#include "stdafx.h"
#include "owca_functions.h"
#include "owca_vm.h"
#include "vm.h"
#include "runtime_function.h"

namespace OwcaScript {
	std::string_view OwcaFunctions::name() const
	{
		return functions->name;
	}
}