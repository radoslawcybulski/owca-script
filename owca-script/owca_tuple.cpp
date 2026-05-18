#include "stdafx.h"
#include "owca_tuple.h"
#include "tuple.h"
#include "owca_value.h"
#include "vm.h"

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
    OwcaValue OwcaTuple::operator + (OwcaTuple other) const {
        return object->vm->create_tuple(*this, other);
    }
    OwcaValue OwcaTuple::operator * (Number other) const {
        return object->vm->create_tuple(*this, other);
    }
    OwcaValue operator * (Number left, OwcaTuple right) {
        return right.object->vm->create_tuple(right, left);
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

    bool OwcaTuple::operator == (OwcaTuple other) const {
        if (size() != other.size()) return false;
        for (size_t i = 0; i < size(); ++i) {
            if (!Internal::VM::get(internal_value()->vm).compare_values(Internal::CompareKind::Eq, internal_value()->values[i], other.internal_value()->values[i])) return false;
        }
        return true;
    }
    
    void gc_mark_value(const OwcaVM &vm, GenerationGC gc, const OwcaTuple &t) {
        gc_mark_value(vm, gc, t.object);
    }
}
