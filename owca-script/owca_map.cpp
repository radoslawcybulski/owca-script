#include "stdafx.h"
#include "owca_map.h"
#include "dictionary.h"
#include "allocation_base.h"
#include "owca_value.h"
#include "generator.h"
#include "vm.h"

namespace OwcaScript {
	std::string OwcaMap::to_string() const
	{
		return dictionary->dict.to_string();
	}

	size_t OwcaMap::size() const
	{
		return dictionary->dict.elements;
	}

	OwcaValue &OwcaMap::operator [] (OwcaValue key)
	{
		return dictionary->dict.item(key);
	}

	OwcaValue OwcaMap::operator [] (OwcaValue key) const
	{
		return dictionary->dict.item(key);
	}

	OwcaMap::Iterator::Iterator(Internal::DictionaryShared *dictionary, size_t pos) : dictionary(dictionary), pos(pos), version(dictionary->dict.version) {}

	OwcaMap::Iterator::reference OwcaMap::Iterator::operator*() const
	{
		auto val = dictionary->dict.read(pos);
		return { *val.first, *val.second };
	}

	OwcaMap::Iterator::pointer OwcaMap::Iterator::operator->()
	{
		auto val = dictionary->dict.read(pos);
		return Pointer{ { *val.first, *val.second } };
	}

	OwcaMap::Iterator& OwcaMap::Iterator::operator++()
	{
		if (dictionary->dict.version != version) {
			dictionary->vm->throw_dictionary_changed(true);
		}
		auto p = dictionary->dict.next(pos);
		pos = std::min(p, dictionary->dict.values.size());
		return *this;
	}

	OwcaMap::Iterator OwcaMap::begin() const
	{
		auto pos = dictionary->dict.next((size_t)-1);
		return Iterator{ dictionary, std::min(pos, dictionary->dict.values.size()) };
	}

	OwcaMap::Iterator OwcaMap::end() const
	{
		return Iterator{ dictionary, dictionary->dict.values.size() };
	}

	OwcaValue *OwcaMap::value(OwcaValue key) const
	{
		auto p = dictionary->dict.find(key);
		if (!p) return nullptr;
		return p->second;
	}

	OwcaValue OwcaMap::pop(OwcaValue key) {
		return dictionary->dict.pop(key);
	}
	OwcaValue OwcaMap::pop_or_default(OwcaValue key, OwcaValue default_value) {
		return dictionary->dict.pop(key, default_value);
	}

	OwcaValue OwcaMap::has_key(OwcaValue key) {
		auto v = dictionary->dict.find(key);
		return bool(v);
	}

	Generator OwcaMap::keys() const {
		return dictionary->dict.iter_keys();
	}
	Generator OwcaMap::values() const {
		return dictionary->dict.iter_values();
	}
	Generator OwcaMap::items() const {
		return dictionary->dict.iter_items();
	}

	OwcaValue OwcaMap::get_or_default(OwcaValue key, OwcaValue default_value) {
		return dictionary->dict.get_or_default(key, default_value);
	}

	OwcaValue OwcaMap::set_default(OwcaValue key, OwcaValue default_value) {
		return dictionary->dict.set_default(key, default_value);
	}

	void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaMap &map) {
		gc_mark_value(vm, gc, map.dictionary);

	}
}
