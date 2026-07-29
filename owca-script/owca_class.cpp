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

	OwcaValue OwcaClass::operator [] (std::string_view key) const
	{
		return Internal::current_vm().member(*this, key);
	}

	OwcaValue OwcaClass::operator [] (IdentifierIndex key) const
	{
		return Internal::current_vm().member(*this, key);
	}

	bool OwcaClass::has_base_class(OwcaClass base) const
	{
		auto it = object->all_base_classes.find(base.internal_value());
		return it != object->all_base_classes.end();
	}

	void gc_mark_value(GenerationGC gc, const OwcaClass &c) {
		gc_mark_value(gc, c.object);
	}
}