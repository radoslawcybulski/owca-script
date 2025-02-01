#include "stdafx.h"
#include "owca_map.h"
#include "dictionary.h"
#include "allocation_base.h"
#include "owca_value.h"

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

	const OwcaValue &OwcaMap::operator [] (OwcaValue key) const
	{
		return dictionary->dict.item(key);
	}

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
}
