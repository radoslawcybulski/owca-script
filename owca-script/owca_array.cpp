#include "stdafx.h"
#include "owca_array.h"
#include "array.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript {
    size_t OwcaArray::size() const
    {
        return object->values.size();
    }

    void OwcaArray::resize(size_t s)
    {
        object->values.resize(s);
    }

    OwcaValue OwcaArray::operator [] (size_t s) const
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

    void OwcaArray::push_back(OwcaValue v)
    {
        object->values.push_back(v);
    }
    void OwcaArray::push_front(OwcaValue v)
    {
        object->values.push_front(v);
    }
    OwcaValue OwcaArray::pop_back()
    {
        if (object->values.empty())
            object->vm->throw_container_is_empty();
        auto v = object->values.back();
        object->values.pop_back();
        return v;
    }
    OwcaValue OwcaArray::pop_front()
    {
        if (object->values.empty())
            object->vm->throw_container_is_empty();
        auto v = object->values.front();
        object->values.pop_front();
        return v;
    }

}
