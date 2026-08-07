#include "owca-script/identifier_index.h"
#include "stdafx.h"
#include "owca_object.h"
#include "owca_class.h"
#include "object.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript {
	void PtrObjectInterface::acquire() {
		if (!generation_gc_) {
			generation_gc_ = Internal::current_vm().get_current_generation_gc();
			Internal::current_vm().register_active_ptr_object_interface(this);
			acquired();
		}
	}
	void PtrObjectInterface::release() {
		if (generation_gc_) {
			generation_gc_ = GenerationGC{};
			released();
		}
	}

	OwcaValue PtrObjectInterface::bound_function_self_object() {
		return OwcaPtrObject{ this };
	}

	bool PtrObjectInterface::get_member(IdentifierIndex, OwcaValue &) {
		return false;
	}

	bool PtrObjectInterface::set_member(IdentifierIndex, OwcaValue) {
		return false;
	}


	void gc_mark_members(PtrObjectInterface &obj, GenerationGC generation_gc) {
		if (obj.generation_gc_ == generation_gc) return;
		obj.generation_gc_ = generation_gc;
		obj.gc_mark_members_impl(generation_gc);
	}

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
