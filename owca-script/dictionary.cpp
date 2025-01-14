#include "stdafx.h"
#include "dictionary.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript::Internal {
	static constexpr const size_t Deleted = 1;
	static constexpr const size_t Empty = 0;
	static constexpr const size_t minimum_size = 16;
	static constexpr const float max_allocation = 0.7f, min_allocation = 0.25f;

	std::tuple<size_t, size_t, bool> Dictionary::find_place(OwcaVM &vm, const OwcaValue &key) const
	{
		auto hash = VM::get(vm).calculate_hash(key);
		if (hash == Deleted || hash == Empty) hash += 2;
		return find_place(vm, key, hash);
	}

	std::tuple<size_t, size_t, bool> Dictionary::find_place(OwcaVM &vm, const OwcaValue &key, size_t hash) const
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

	OwcaValue& Dictionary::item(OwcaVM &vm, const OwcaValue &key)
	{
		try_rehash();
		auto [index, hash, exists] = find_place(vm, key);
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
	const OwcaValue& Dictionary::read(OwcaVM &vm, const OwcaValue &key)
	{
		auto [index, hash, exists] = find_place(vm, key);
		if (!exists) {
			VM::get(vm).throw_missing_key(key.to_string());
		}
		return std::get<2>(values[index]);
	}

	void Dictionary::write(OwcaVM &vm, const OwcaValue &key, OwcaValue value)
	{
		item(vm, key) = std::move(value);
	}

	OwcaValue Dictionary::pop(OwcaVM &vm, const OwcaValue &key)
	{
		auto [index, hash, exists] = find_place(vm, key);
		if (!exists) {
			VM::get(vm).throw_missing_key(key.to_string());
		}
		--elements;
		std::get<0>(values[index]) = Deleted;
		auto v = std::get<2>(values[index]);
		try_rehash();
		return v;
	}

	std::optional<std::pair<OwcaValue, OwcaValue>> Dictionary::find(OwcaVM &vm, const OwcaValue &key) const
	{
		auto [i, hash, exists] = find_place(vm, key);
		if (!exists) return std::nullopt;
		return std::make_pair(std::get<1>(values[i]), std::get<2>(values[i]));
	}
	std::optional<std::pair<OwcaValue, OwcaValue>> Dictionary::read_and_update_iterator(Iterator& iter) const
	{
		while (iter.pos < values.size()) {
			auto i = iter.pos++;
			auto h = std::get<0>(values[i]);
			if (h != Empty && h != Deleted) {
				return std::make_pair(std::get<1>(values[i]), std::get<2>(values[i]));
			}
		}
		return std::nullopt;
	}

	std::string_view Dictionary::type() const
	{
		return "dictionary";
	}

	std::string Dictionary::to_string() const
	{
		std::ostringstream tmp;
		bool first = true;

		tmp << "{";
		
		Iterator it;
		while(true) {
			auto v = read_and_update_iterator(it);
			if (!v) break;
			
			if (first) first = false;
			else tmp << ",";
			tmp << " " << v->first.to_string() << ": ";
			tmp << v->second.to_string();
		}
		tmp << " }";
		return tmp.str();

	}
	void Dictionary::gc_mark(VM& vm, GenerationGC generation_gc)
	{
		Iterator it;
		while (true) {
			auto v = read_and_update_iterator(it);
			if (!v) break;
			vm.gc_mark(v->first, generation_gc);
			vm.gc_mark(v->second, generation_gc);
		}
	}

}