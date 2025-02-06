#include "stdafx.h"
#include "dictionary.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript::Internal {
	static constexpr const size_t Deleted = 1;
	static constexpr const size_t Empty = 0;
	static constexpr const size_t minimum_size = 16;
	static constexpr const float max_allocation = 0.7f, min_allocation = 0.25f;

	std::tuple<size_t, size_t, bool> Dictionary::find_place(const OwcaValue &key) const
	{
		auto hash = VM::get(vm).calculate_hash(key);
		if (hash == Deleted || hash == Empty) hash += 2;
		return find_place(key, hash);
	}

	std::tuple<size_t, size_t, bool> Dictionary::find_place(const OwcaValue &key, size_t hash) const
	{
		if (values.empty()) return { hash & (minimum_size - 1), hash, false};

		size_t index = hash & mask;

		for (auto i = 0u; i < values.size(); ++i, index = (index + i + 1) & mask) {
			auto& p = values[index];
			auto cell_hash = std::get<0>(p);
			if (cell_hash == Empty) return {index, hash, false};
			if (cell_hash == Deleted) continue;
			if (cell_hash != hash) continue;
			if (VM::get(vm).compare_values(std::get<1>(p), key)) return { index, hash, true };
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

	OwcaValue& Dictionary::item(const OwcaValue &key)
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
			++elements;
		}
		return std::get<2>(values[index]);
	}
	const OwcaValue& Dictionary::read(const OwcaValue &key)
	{
		auto [index, hash, exists] = find_place(key);
		if (!exists) {
			VM::get(vm).throw_missing_key(key.to_string());
		}
		return std::get<2>(values[index]);
	}

	void Dictionary::write(const OwcaValue &key, OwcaValue value)
	{
		item(key) = std::move(value);
	}

	OwcaValue Dictionary::pop(const OwcaValue &key)
	{
		auto [index, hash, exists] = find_place(key);
		if (!exists) {
			VM::get(vm).throw_missing_key(key.to_string());
		}
		--elements;
		std::get<0>(values[index]) = Deleted;
		auto v = std::get<2>(values[index]);
		try_rehash();
		return v;
	}

	std::optional<std::pair<const OwcaValue*, OwcaValue*>> Dictionary::find(const OwcaValue &key)
	{
		auto [i, hash, exists] = find_place(key);
		if (!exists) return std::nullopt;
		return std::pair<const OwcaValue*, OwcaValue*>{ &std::get<1>(values[i]), &std::get<2>(values[i]) };
	}

	std::optional<std::pair<const OwcaValue*, const OwcaValue*>> Dictionary::find(const OwcaValue &key) const
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
	void Dictionary::gc_mark(VM& vm, GenerationGC generation_gc)
	{
		for(auto pos = next(); pos < values.size(); pos = next(pos)) {
			auto v = read(pos);
			vm.gc_mark(*v.first, generation_gc);
			vm.gc_mark(*v.second, generation_gc);
		}
	}
}