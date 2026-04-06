#include "stdafx.h"
#include "dictionary.h"
#include "owca_value.h"
#include "generator.h"
#include "vm.h"

namespace OwcaScript::Internal {
	static constexpr const size_t Deleted = 1;
	static constexpr const size_t Empty = 0;
	static constexpr const size_t AllocatedMin = 2;
	static constexpr const size_t minimum_size = 16;
	static constexpr const float max_allocation = 0.7f, min_allocation = 0.25f;

	std::tuple<size_t, size_t, bool> Dictionary::find_place(OwcaValue key) const
	{
		auto hash = VM::get(vm).calculate_hash(key);
		if (hash == Deleted || hash == Empty) hash += 2;
		return find_place(key, hash);
	}

	std::tuple<size_t, size_t, bool> Dictionary::find_place(OwcaValue key, size_t hash) const
	{
		if (values.empty()) return { hash & (minimum_size - 1), hash, false};

		size_t index = hash & mask;

		for (auto i = 0u; i < values.size(); ++i, index = (index + i + 1) & mask) {
			auto& p = values[index];
			auto cell_hash = std::get<0>(p);
			if (cell_hash == Empty) return {index, hash, false};
			if (cell_hash == Deleted) continue;
			if (cell_hash != hash) continue;
			if (VM::get(vm).compare_values(CompareKind::Eq, std::get<1>(p), key)) return { index, hash, true };
		}
		assert(false);
		return { 0, 0, false };
	}

	void Dictionary::try_rehash()
	{
		auto new_size = values.size();
		while (elements > new_size * max_allocation) new_size *= 2;
		while (elements > minimum_size && elements < new_size * min_allocation) new_size /= 2;

		if (new_size != values.size()) {
			++version;
			auto old_values = std::move(values);
			values.resize(new_size);
			mask = new_size - 1;
			assert((mask & new_size) == 0);

			for (auto i = 0u; i < old_values.size(); ++i) {
				auto hash = std::get<0>(old_values[i]);
				if (hash != Deleted && hash != Empty) {
					size_t index = hash & mask;
					size_t j;
					for (j = 0u; j < values.size(); ++j, index = (index + j + 1) & mask) {
						auto& p = values[index];
						auto new_hash = std::get<0>(p);
						if (new_hash == Empty) {
							p = std::move(old_values[i]);
							break;
						}
						if (new_hash == Deleted) assert(false);
					}
					assert(j < values.size());
				}
			}
		}
	}

	OwcaValue& Dictionary::item(OwcaValue key)
	{
		try_rehash();
		auto [index, hash, exists] = find_place(key);
		if (!exists) {
			if (values.empty()) {
				values.resize(minimum_size);
				mask = minimum_size - 1;
			}
			std::get<0>(values[index]) = hash;
			std::get<1>(values[index]) = key;
			std::get<2>(values[index]) = {};
			++version;
			++elements;
		}
		return std::get<2>(values[index]);
	}
	OwcaValue Dictionary::read(OwcaValue key)
	{
		auto [index, hash, exists] = find_place(key);
		if (!exists) {
			VM::get(vm).throw_missing_key(key.to_string());
		}
		return std::get<2>(values[index]);
	}

	void Dictionary::write(OwcaValue key, OwcaValue value)
	{
		item(key) = std::move(value);
	}

	void Dictionary::delete_pos(size_t pos, bool rehash)
	{
		std::get<0>(values[pos]) = Deleted;
		if (rehash) try_rehash();
		++version;
		--elements;
	}	
	OwcaValue Dictionary::pop(OwcaValue key, std::optional<OwcaValue> default_value)
	{
		auto [index, hash, exists] = find_place(key);
		if (!exists) {
			if (default_value.has_value()) {
				return *default_value;
			}
			VM::get(vm).throw_missing_key(key.to_string());
		}
		auto v = std::get<2>(values[index]);
		delete_pos(index);
		return v;
	}

	OwcaValue Dictionary::get_or_default(OwcaValue key, OwcaValue default_value)
	{
		auto [index, hash, exists] = find_place(key);
		if (!exists) {
			return default_value;
		}
		return std::get<2>(values[index]);
	}

	OwcaValue Dictionary::set_default(OwcaValue key, OwcaValue default_value)
	{
		auto [index, hash, exists] = find_place(key);
		if (!exists) {
			if (values.empty()) {
				values.resize(minimum_size);
				mask = minimum_size - 1;
			}
			std::get<0>(values[index]) = hash;
			std::get<1>(values[index]) = key;
			std::get<2>(values[index]) = default_value;
			++version;
			++elements;
		}
		return std::get<2>(values[index]);
	}

	Generator Dictionary::iter_keys() const { // AllocatedMin
		size_t ver = version;
		for(auto &q : values) {
			if (ver != version) Internal::VM::get(vm).throw_dictionary_changed(is_map);
			if (std::get<0>(q) < AllocatedMin) continue;
			co_yield std::get<1>(q);
		}
	}
	Generator Dictionary::iter_values() const {
		size_t ver = version;
		for(auto &q : values) {
			if (ver != version) Internal::VM::get(vm).throw_dictionary_changed(is_map);
			if (std::get<0>(q) < AllocatedMin) continue;
			co_yield std::get<2>(q);
		}
	}
	Generator Dictionary::iter_items() const {
		size_t ver = version;
		for(auto &q : values) {
			if (ver != version) Internal::VM::get(vm).throw_dictionary_changed(is_map);
			if (std::get<0>(q) < AllocatedMin) continue;
			auto t = Internal::VM::get(vm).create_tuple(std::pair{ std::get<1>(q), std::get<2>(q) });
			co_yield t;
		}
	}
	void Dictionary::clone_to(Dictionary &dest) const {
		dest.values = values;
		dest.elements = elements;
		dest.mask = mask;
		dest.version = version;
	}
	DictionaryShared *DictionaryShared::clone() const {
		auto new_shared = Internal::VM::get(vm).allocate<DictionaryShared>(0, vm);
		dict.clone_to(new_shared->dict);
		return new_shared;
	}
	SetShared *SetShared::clone() const {
		auto new_shared = Internal::VM::get(vm).allocate<SetShared>(0, vm);
		dict.clone_to(new_shared->dict);
		return new_shared;
	}
	void Dictionary::union_with(Dictionary &other) {
		for(auto pos = other.next(); pos < other.values.size(); pos = other.next(pos)) {
			auto v = other.read(pos);
			write(*v.first, *v.second);
		}		
	}
	void Dictionary::intersection_with(Dictionary &other) {
		for(auto pos = next(); pos < values.size(); pos = next(pos)) {
			auto v = read(pos);
			if (!other.find(*v.first)) {
				delete_pos(pos, false);
			}
		}
		try_rehash();
	}
	void Dictionary::difference_with(Dictionary &other) {
		for(auto pos = next(); pos < values.size(); pos = next(pos)) {
			auto v = read(pos);
			if (other.find(*v.first)) {
				delete_pos(pos, false);
			}
		}
		try_rehash();
	}

	std::optional<std::pair<const OwcaValue*, OwcaValue*>> Dictionary::find(OwcaValue key)
	{
		auto [i, hash, exists] = find_place(key);
		if (!exists) return std::nullopt;
		return std::pair<const OwcaValue*, OwcaValue*>{ &std::get<1>(values[i]), &std::get<2>(values[i]) };
	}

	std::optional<std::pair<const OwcaValue*, const OwcaValue*>> Dictionary::find(OwcaValue key) const
	{
		auto [i, hash, exists] = find_place(key);
		if (!exists) return std::nullopt;
		return std::pair<const OwcaValue*, const OwcaValue*>{ &std::get<1>(values[i]), &std::get<2>(values[i]) };
	}

	size_t Dictionary::next(size_t pos) const
	{
		++pos;
		while(pos < values.size()) {
			auto h = std::get<0>(values[pos]);
			if (h != Empty && h != Deleted) {
				break;
			}
			++pos;
		}
		return pos;
	}
	std::pair<const OwcaValue *, OwcaValue *> Dictionary::read(size_t pos)
	{
		auto h = std::get<0>(values[pos]);
		assert(h != Empty && h != Deleted);
		return std::pair<const OwcaValue*, OwcaValue*>{ &std::get<1>(values[pos]), &std::get<2>(values[pos]) };
	}
	std::pair<const OwcaValue *, const OwcaValue *> Dictionary::read(size_t pos) const
	{
		auto h = std::get<0>(values[pos]);
		assert(h != Empty && h != Deleted);
		return std::pair<const OwcaValue*, const OwcaValue*>{ &std::get<1>(values[pos]), &std::get<2>(values[pos]) };
	}

	std::string_view Dictionary::type() const
	{
		return is_map ? "Map" : "Set";
	}

	std::string Dictionary::to_string() const
	{
		std::ostringstream tmp;
		bool first = true;

		tmp << "{";
		
		for(auto pos = next(); pos < values.size(); pos = next(pos)) {
			auto v = read(pos);
			
			if (first) first = false;
			else tmp << ",";
			tmp << " " << v.first->to_string();
			if (is_map) 
				tmp << ": " << v.second->to_string();
		}
		tmp << " }";
		return tmp.str();

	}
	void gc_mark_value(OwcaVM vm, GenerationGC gc, const Dictionary &d)
	{
		for(auto pos = d.next(); pos < d.values.size(); pos = d.next(pos)) {
			auto v = d.read(pos);
			gc_mark_value(vm, gc, *v.first);
			gc_mark_value(vm, gc, *v.second);
		}
	}
}