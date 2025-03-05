#include "stdafx.h"
#include "owca_code.h"
#include "code_buffer.h"

namespace OwcaScript {
    std::vector<unsigned char> OwcaCode::serialize_to_binary() const
    {
        return code_->serialize();
    }

    bool OwcaCode::compare(const OwcaCode &other) const
    {
        return code_->compare(*other.code_);
    }
}