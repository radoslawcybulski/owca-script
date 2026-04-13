#include "stdafx.h"
#include "exec_buffer.h"

namespace OwcaScript::Internal {
    ExecuteBufferReader::ExecuteBufferReader(OwcaCode code, size_t pos) : code_object_(std::move(code)), 
        code_(code_object_.code()), data_kinds(code_object_.data_kinds()), lines(code_object_.lines()), pos(pos) {
    }

    Line ExecuteBufferReader::line() const {
        auto it = std::lower_bound(lines.begin(), lines.end(), pos, [](const LineEntry &entry, size_t pos) {
            return entry.code_pos <= pos;
        });
        if (it == lines.end()) {
            return Line{ 0 };
        }
        return Line{ it->line };
    }
}