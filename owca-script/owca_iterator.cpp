#include "stdafx.h"
#include "owca_iterator.h"
#include "owca_value.h"
#include "iterator.h"

namespace OwcaScript {
    bool OwcaIterator::completed() const
    {
        return !bool(object->generator);
    }
    OwcaValue OwcaIterator::next() const
    {
        return object->execute_next();
    }
    OwcaIterator::Iterator& OwcaIterator::Iterator::operator++() {
        iter->next();
        return *this;
    }
    OwcaIterator::Iterator::Iterator(OwcaIterator *iter) : iter(iter) {
        if (iter && iter->object->first_time) {
            iter->object->execute_next();
        }
    }

    OwcaIterator::Iterator::reference OwcaIterator::Iterator::operator*() const { return iter->object->value; }
    OwcaIterator::Iterator::pointer OwcaIterator::Iterator::operator->() const { return &iter->object->value; }

    void gc_mark_value(OwcaVM vm, GenerationGC gc, const OwcaIterator &o) {
        gc_mark_value(vm, gc, o.object);
    }
}