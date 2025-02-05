#include "stdafx.h"
#include "owca_array.h"
#include "array.h"
#include "owca_value.h"

namespace OwcaScript {
    size_t OwcaArray::size() const
    {
        return object->values.size();
    }

    void OwcaArray::resize(size_t s)
    {
        object->values.resize(s);
    }

    void OwcaArray::reserve(size_t s)
    {
        object->values.reserve(s);
    }

    const OwcaValue &OwcaArray::operator [] (size_t s) const
    {
        assert(s < object->values.size());
        return object->values[s];
    }

    OwcaValue &OwcaArray::operator [] (size_t s)
    {
        assert(s < object->values.size());
        return object->values[s];
    }

    std::string OwcaArray::to_string() const
    {
        return object->to_string();
    }

    std::vector<OwcaValue>::iterator OwcaArray::begin() { return object->values.begin(); }
    std::vector<OwcaValue>::iterator OwcaArray::end() { return object->values.end(); }
    std::vector<OwcaValue>::const_iterator OwcaArray::cbegin() { return object->values.cbegin(); }
    std::vector<OwcaValue>::const_iterator OwcaArray::cend() { return object->values.cend(); }
}
