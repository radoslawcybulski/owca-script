#include "stdafx.h"
#include "owca_tuple.h"
#include "tuple.h"
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

    void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaTuple &t) {
        gc_mark_value(vm, gc, t.object);
    }
}
