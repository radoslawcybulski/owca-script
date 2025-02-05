#include "stdafx.h"
#include "owca_string.h"
#include "owca_value.h"

namespace OwcaScript {
    OwcaString OwcaString::substr(size_t from, size_t to) const
    {
        if (to > value.size()) to = value.size();
        if (from > to) return OwcaString{ "" };
        return OwcaString{ value.substr(from, to - from) };
    }

    OwcaString OwcaString::operator [] (size_t pos) const
    {
        assert(pos < value.size());
        return OwcaString{ value.substr(pos, 1) };
    }

    size_t OwcaString::size() const
    {
        return value.size();
    }
}
