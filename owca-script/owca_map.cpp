#include "stdafx.h"
#include "owca_map.h"
#include "dictionary.h"
#include "allocation_base.h"

namespace OwcaScript {
	std::string OwcaMap::to_string() const
	{
		return dictionary->dict.to_string();
	}

	size_t OwcaMap::size() const
	{
		return dictionary->dict.elements;
	}
}