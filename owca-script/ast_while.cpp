#include "stdafx.h"
#include "ast_while.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
    void AstWhile::emit(EmitInfo& ei) {
        auto continue_position = ei.code_writer.position();
        auto condition = value_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::IfAlmostAlwaysFalse);
        ei.code_writer.append(line, condition.index);
        const auto end_pos = ei.code_writer.append_jump_placeholder(line);

        ei.break_loops.push_back({ .depth=loop_control_depth_ });
        body_->emit(ei);
        auto break_loop = std::move(ei.break_loops.back());
        ei.break_loops.pop_back();
        ei.code_writer.append(line, ExecuteOp::Jump);
        ei.code_writer.append_jump_position(line, continue_position);
        ei.code_writer.update_jump_placeholder(end_pos, (std::int32_t)ei.code_writer.position());

        for(auto & break_pos : break_loop.break_positions) {
            ei.code_writer.update_jump_placeholder(break_pos, ei.code_writer.position());
        }
        for(auto & continue_pos : break_loop.continue_positions) {
            ei.code_writer.update_jump_placeholder(continue_pos, continue_position);
        }
    }

    void AstWhile::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstWhile::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        body_->visit(vis);
    }
}