#include "owca-script/identifier_index.h"
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

	OwcaValue OwcaObject::member(std::string_view key) const
	{
		return Internal::current_vm().member(*this, key);
	}
	std::optional<OwcaValue> OwcaObject::try_member(std::string_view key) const
	{
		return Internal::current_vm().try_member(*this, key);
	}
	void OwcaObject::member(std::string_view key, OwcaValue val)
	{
		Internal::current_vm().member(*this, key, std::move(val));
	}

	OwcaValue OwcaObject::member(IdentifierIndex key) const
	{
		return Internal::current_vm().member(*this, key);
	}
	std::optional<OwcaValue> OwcaObject::try_member(IdentifierIndex key) const
	{
		return Internal::current_vm().try_member(*this, key);
	}
	void OwcaObject::member(IdentifierIndex key, OwcaValue val)
	{
		Internal::current_vm().member(*this, key, std::move(val));
	}

	std::span<char> OwcaObject::user_data_impl(Internal::UserClassTokenPtr token) const
	{
		return object->native_storage_raw(token);
	}
	void gc_mark_value(GenerationGC gc, const OwcaObject &obj)
	{
		gc_mark_value(gc, obj.internal_value());
	}
}
