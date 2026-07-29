#include "owca-script/identifier_index.h"
#include "stdafx.h"
#include "owca_namespace.h"
#include "namespace.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript {
	std::string OwcaNamespace::to_string() const
	{
		return object->to_string();
	}
	std::string_view OwcaNamespace::type() const
	{
		return object->type();
	}

	OwcaValue OwcaNamespace::member(std::string_view key) const
	{
		return object->member(key);
	}
	std::optional<OwcaValue> OwcaNamespace::try_member(std::string_view key) const
	{
		return object->try_member(key);
	}
	void OwcaNamespace::member(std::string_view key, OwcaValue val)
	{
		object->set_member(key, std::move(val));
	}
    bool OwcaNamespace::try_member(std::string_view key, OwcaValue val) {
        return object->try_set_member(key, std::move(val));
    }

    OwcaValue OwcaNamespace::member(IdentifierIndex key) const
	{
		return object->member(key);
	}
	std::optional<OwcaValue> OwcaNamespace::try_member(IdentifierIndex key) const
	{
		return object->try_member(key);
	}
	void OwcaNamespace::member(IdentifierIndex key, OwcaValue val)
	{
		object->set_member(key, std::move(val));
	}
       bool OwcaNamespace::try_member(IdentifierIndex key, OwcaValue val) {
           return object->try_set_member(key, std::move(val));
    }

    OwcaNamespace::Iterator::Iterator(Internal::Namespace *nspace, std::unordered_map<IdentifierIndex, size_t>::const_iterator it) : nspace(nspace), it(it) {}

	OwcaNamespace::Iterator::reference OwcaNamespace::Iterator::operator*() const
	{
		auto val = *it;
		return { val.first, nspace->globals[val.second] };

	}

	OwcaNamespace::Iterator::pointer OwcaNamespace::Iterator::operator->()
	{
		auto val = *it;
		return Pointer{ { val.first, nspace->globals[val.second] } };
	}

	OwcaNamespace::Iterator& OwcaNamespace::Iterator::operator++()
	{
		++it;
		return *this;
	}

	OwcaNamespace::Iterator OwcaNamespace::begin() const
	{
		auto it = object->code.identifier_to_global_index().begin();
		return Iterator{ object, it };
	}

	OwcaNamespace::Iterator OwcaNamespace::end() const
	{
		auto it = object->code.identifier_to_global_index().end();
		return Iterator{ object, it };
	}
	void gc_mark_value(GenerationGC gc, OwcaNamespace obj)
	{
		gc_mark_value(gc, obj.internal_value());
	}
}
