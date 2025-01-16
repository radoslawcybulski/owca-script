#include "stdafx.h"
#include "owca_object.h"
#include "object.h"
#include "vm.h"

namespace OwcaScript {
	std::string OwcaObject::to_string() const
	{
		return object->to_string();
	}
	std::string_view OwcaObject::type() const
	{
		return object->type();
	}
	
	OwcaValue OwcaObject::member(OwcaVM &vm, const std::string& key) const
	{
		auto val = object->lookup(key);
		if (!val) {
			Internal::VM::get(vm).throw_missing_member(object->type(), key);
		}
		return std::move(*val);
	}
	void OwcaObject::member(const std::string& key, OwcaValue val)
	{
		object->values[key] = std::move(val);
	}
}