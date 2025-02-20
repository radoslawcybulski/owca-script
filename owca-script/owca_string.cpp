#include "stdafx.h"
#include "owca_string.h"
#include "owca_value.h"
#include "string.h"
#include "vm.h"

namespace OwcaScript {
    OwcaValue OwcaString::substr(size_t start, size_t end) const
    {
        return str->vm->create_string(*this, start, end);
    }

    OwcaValue OwcaString::operator [] (size_t pos) const
    {
        return str->vm->create_string(*this, pos, 1);
    }

    size_t OwcaString::size() const
    {
        return str->size();
    }

    size_t OwcaString::hash() const
    {
        return str->hash();
    }

    std::string_view OwcaString::text() const
    {
        return str->text();
    }
}
