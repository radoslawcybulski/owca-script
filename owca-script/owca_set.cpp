#include "stdafx.h"
#include "owca_set.h"
#include "dictionary.h"
#include "allocation_base.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript {
	std::string OwcaSet::to_string() const
	{
		return dictionary->dict.to_string();
	}

	size_t OwcaSet::size() const
	{
		return dictionary->dict.elements;
	}

	OwcaSet::Iterator::Iterator(Internal::SetShared *dictionary, size_t pos) : dictionary(dictionary), pos(pos), version(dictionary->dict.version) {}
	
	OwcaSet::Iterator::reference OwcaSet::Iterator::operator*() const
	{
		auto val = dictionary->dict.read(pos);
		return *val.first;
	}

	OwcaSet::Iterator::pointer OwcaSet::Iterator::operator->()
	{
		auto val = dictionary->dict.read(pos);
		return val.first;
	}

	OwcaSet::Iterator& OwcaSet::Iterator::operator++()
	{
		if (dictionary->dict.version != version) {
			dictionary->vm->throw_dictionary_changed(false);
		}
		auto p = dictionary->dict.next(pos);
		pos = std::min(p, dictionary->dict.values.size());
		return *this;
	}

	OwcaSet::Iterator OwcaSet::begin() const
	{
		auto pos = dictionary->dict.next((size_t)-1);
		return Iterator{ dictionary, std::min(pos, dictionary->dict.values.size()) };
	}

	OwcaSet::Iterator OwcaSet::end() const
	{
		return Iterator{ dictionary, dictionary->dict.values.size() };
	}

	bool OwcaSet::has_value(OwcaValue key) const
	{
		auto p = dictionary->dict.find(key);
        return bool(p);
	}

	void OwcaSet::add(OwcaValue key) {
		dictionary->dict.item(key) = {};
	}
	void OwcaSet::remove(OwcaValue key) {
		dictionary->dict.pop(key);
	}
	OwcaSet OwcaSet::copy() const {
		return OwcaSet{ dictionary->clone() };
	}
	void OwcaSet::union_with(OwcaSet other) {
		dictionary->dict.union_with(other.internal_value()->dict);
	}
	void OwcaSet::intersection_with(OwcaSet other) {
		dictionary->dict.intersection_with(other.internal_value()->dict);
	}
	void OwcaSet::difference_with(OwcaSet other) {
		dictionary->dict.difference_with(other.internal_value()->dict);
	}

	void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaSet &o) {
		gc_mark_value(vm, gc, o.dictionary);
	}
}
