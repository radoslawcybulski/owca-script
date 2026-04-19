#include "stdafx.h"
#include "exec_buffer.h"

namespace OwcaScript::Internal {
    ExecuteBufferReader::ExecuteBufferReader(OwcaCode code, size_t pos) : code_object_(std::move(code)), 
        code_(code_object_.code()), data_kinds(code_object_.data_kinds()), lines(code_object_.lines()), pos(pos) {
    }

    Line ExecuteBufferReader::line_from_code_pos(std::uint32_t pos) const {
        auto it = std::lower_bound(lines.begin(), lines.end(), pos, [](const Internal::LineEntry &entry, size_t pos) {
            return entry.code_pos < pos;
        });
        if (it == lines.end()) {
            if (lines.empty())
                return Internal::Line{ 0 };
            return Internal::Line{ lines.back().line };
        }
        if (it == lines.begin() || it->code_pos == pos) {
            return Internal::Line{ it->line };
        }
        --it;
        return Internal::Line{ it->line };
    }
}