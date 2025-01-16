#include "stdafx.h"
#include "owca_class.h"
#include "object.h"

namespace OwcaScript {
	std::string OwcaClass::to_string() const
	{
		return object->to_string();
	}
}