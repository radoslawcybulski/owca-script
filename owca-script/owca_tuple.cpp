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

    OwcaTuple::Iterator::reference OwcaTuple::Iterator::operator*() const
    {
        return tuple->values[pos];
    }

    OwcaTuple::Iterator::pointer OwcaTuple::Iterator::operator->()
    {
        return &tuple->values[pos];
    }

    void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaTuple &t) {
        gc_mark_value(vm, gc, t.object);
    }
}
