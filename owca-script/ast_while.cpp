#include "stdafx.h"
#include "ast_while.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
    void AstWhile::emit(EmitInfo& ei) {
        assert(ei.stack.empty());
        ei.states.push();
        ei.code_writer.append(line, ExecuteOp::WhileInit);
        const auto end = ei.code_writer.append_jump_placeholder(line);
        ei.code_writer.append(line, loop_ident_index_.value_or(std::numeric_limits<std::uint32_t>::max()));
        ei.code_writer.append(line, loop_control_depth_);
        const auto pos = ei.code_writer.position();
        ei.code_writer.append(line, ExecuteOp::WhileCondition);
        value_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::WhileNext);
        ei.stack.pop();
        assert(ei.stack.empty());
        ei.break_loops.push_back({ .depth=loop_control_depth_ });
        body_->emit(ei);
        auto break_loop = std::move(ei.break_loops.back());
        ei.break_loops.pop_back();
        assert(ei.stack.empty());
        ei.code_writer.append(line, ExecuteOp::Jump);
        ei.code_writer.append_jump_position(line, pos);
        ei.code_writer.update_jump_placeholder(end, (std::int32_t)ei.code_writer.position());

        for(auto & break_pos : break_loop.break_positions) {
            ei.code_writer.update_jump_placeholder(break_pos, ei.code_writer.position());
        }
        for(auto & continue_pos : break_loop.continue_positions) {
            ei.code_writer.update_jump_placeholder(continue_pos, pos);
        }

        ei.code_writer.append(line, ExecuteOp::WhileCompleted);
        ei.states.pop();
    }

    void AstWhile::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstWhile::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        body_->visit(vis);
    }
}