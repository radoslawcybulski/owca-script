#include "stdafx.h"
#include "owca_iterator.h"
#include "owca_value.h"

namespace OwcaScript {
    std::optional<OwcaValue> OwcaIterator::current() const
    {
        auto v = ib->get();
        if (v) return *v;
        return std::nullopt;
    }

    void OwcaIterator::next() {
        ib->next();
    }

    size_t OwcaIterator::remaining_size() const
    {
        return ib->remaining_size();
    }
}