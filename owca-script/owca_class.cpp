#include "stdafx.h"
#include "owca_class.h"
#include "object.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript {
	std::string OwcaClass::to_string() const
	{
		return object->to_string();
	}

	OwcaValue OwcaClass::operator [] (const std::string &key) const
	{
		return object->vm->member(*this, key);
	}
}