#include "stdafx.h"
#include "owca_set.h"
#include "dictionary.h"
#include "allocation_base.h"
#include "owca_value.h"

namespace OwcaScript {
	std::string OwcaSet::to_string() const
	{
		return dictionary->dict.to_string();
	}

	size_t OwcaSet::size() const
	{
		return dictionary->dict.elements;
	}

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

	bool OwcaSet::has_value(const OwcaValue &key) const
	{
		auto p = dictionary->dict.find(key);
        return bool(p);
	}
}
