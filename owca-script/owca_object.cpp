#include "stdafx.h"
#include "owca_object.h"
#include "object.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript {
	std::string OwcaObject::to_string() const
	{
		return object->to_string();
	}
	std::string_view OwcaObject::type() const
	{
		return object->type();
	}
	
	OwcaValue OwcaObject::member(const std::string& key) const
	{
		return object->vm->member(*this, key);
	}
	std::optional<OwcaValue> OwcaObject::try_member(const std::string& key) const
	{
		return object->vm->try_member(*this, key);
	}
	void OwcaObject::member(const std::string& key, OwcaValue val)
	{
		object->vm->member(*this, key, std::move(val));
	}

	std::span<char> OwcaObject::user_data_impl(Internal::UserClassTokenPtr token) const
	{
		return object->native_storage_raw(token);
	}
	void gc_mark_value(const OwcaVM &vm, GenerationGC gc, const OwcaObject &obj)
	{
		gc_mark_value(vm, gc, obj.internal_value());
	}
}