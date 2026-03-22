#include "stdafx.h"
#include "owca_range.h"
#include "owca_value.h"

namespace OwcaScript {
    std::pair<Number, Number> OwcaRange::internal_values() const {
        return { lower_, upper_ };
    }
}