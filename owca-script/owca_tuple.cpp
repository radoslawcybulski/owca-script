#include "stdafx.h"
#include "owca_tuple.h"
#include "array.h"
#include "owca_value.h"

namespace OwcaScript {
    size_t OwcaTuple::size() const
    {
        return object->values.size();
    }

    OwcaValue OwcaTuple::operator [] (size_t s) const
    {
        assert(s < object->values.size());
        return object->values[s];
    }

    std::string OwcaTuple::to_string() const
    {
        return object->to_string();
    }

    std::vector<OwcaValue>::const_iterator OwcaTuple::begin() { return object->values.begin(); }
    std::vector<OwcaValue>::const_iterator OwcaTuple::end() { return object->values.end(); }
}
