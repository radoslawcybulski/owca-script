#include "stdafx.h"
#include "ast_with.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
    void AstWith::emit(EmitInfo& ei) {
        auto temp = ei.allocate_temporary();
        auto src = value_->emit(ei, std::move(temp));
        ei.code_writer.append(line, ExecuteOp::With);
        ei.code_writer.append(line, src.index);
        ei.code_writer.append(line, identifier_index_);
        auto end = ei.code_writer.append_jump_placeholder(line);
        body_->emit(ei);
        ei.code_writer.update_jump_placeholder(end, (std::uint32_t)ei.code_writer.position());
    }

    void AstWith::visit(AstVisitor& vis) { vis.apply(*this); }
    void AstWith::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        body_->visit(vis);
    }
}