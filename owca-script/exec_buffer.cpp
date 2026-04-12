#include "stdafx.h"
#include "exec_buffer.h"

namespace OwcaScript::Internal {
    ExecuteBufferReader::ExecuteBufferReader(OwcaCode code, size_t pos) : code_object_(std::move(code)), 
        code_(code_object_.code()), data_kinds(code_object_.data_kinds()), lines(code_object_.lines()), pos(pos) {
    }
}