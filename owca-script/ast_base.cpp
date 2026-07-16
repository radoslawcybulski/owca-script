#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript::Internal {
    AstBase::TempInfo AstBase::EmitInfo::allocate_temporary() {
        if (!per_function.temporaries.empty()) {
            auto index = per_function.temporaries.back();
            per_function.temporaries.pop_back();
            return TempInfo{ *this, IdentifierIndex{ IdentifierIndexKind::Local, index } };
        }
        auto index = (std::uint32_t)per_function.max_temporaries++;
        return TempInfo{ *this, IdentifierIndex{ IdentifierIndexKind::Local, index } };
    }

    void AstBase::EmitInfo::write_move(Line line, IdentifierIndex dest, IdentifierIndex src) {
        code_writer.append(line, ExecuteOp::ExprMove);
        code_writer.append(line, dest);
        code_writer.append(line, src);
    }
}