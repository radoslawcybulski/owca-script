#include "stdafx.h"
#include "owca_code.h"
#include "exec_buffer.h"

namespace OwcaScript {
    Internal::Line OwcaCode::get_line_by_position(size_t pos) const
    {
        auto it = std::lower_bound(code_->lines.begin(), code_->lines.end(), pos, [](const Internal::LineEntry &entry, size_t pos) {
            return entry.code_pos < pos;
        });
        if (it == code_->lines.end()) {
            if (code_->lines.empty())
                return Internal::Line{ 0 };
            return Internal::Line{ code_->lines.back().line };
        }
        if (it == code_->lines.begin() || it->code_pos == pos) {
            return Internal::Line{ it->line };
        }
        --it;
        return Internal::Line{ it->line };
    }
}