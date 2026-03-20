#include "stdafx.h"
#include "owca_range.h"
#include "owca_value.h"

namespace OwcaScript {
    OwcaFloat OwcaRange::lower() const { return OwcaFloat{ lower_ }; }
    OwcaFloat OwcaRange::upper() const { return OwcaFloat{ upper_ }; }

    std::pair<OwcaNumberUnderlying, OwcaNumberUnderlying> OwcaRange::internal_values() const {
        return { lower_, upper_ };
    }
}