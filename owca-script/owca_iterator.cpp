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
}