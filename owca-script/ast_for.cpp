#include "stdafx.h"
#include "ast_for.h"
#include "variable_index.h"
#include "ast_expr_noop.h"

namespace OwcaScript::Internal {
    void AstFor::emit(EmitInfo& ei) {
        auto iterator_temp = ei.allocate_temporary();

        auto iterator = iterator_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::ExprToIterator);
        ei.code_writer.append(line, iterator_temp.index);
        ei.code_writer.append(line, iterator.index);

        auto continue_position = ei.code_writer.position();
        auto iter_value = ei.allocate_temporary();
        ei.code_writer.append(line, ExecuteOp::ExprIteratorNextAndJumpIfCompleted);
        ei.code_writer.append(line, iter_value.index);
        ei.code_writer.append(line, iterator_temp.index);
        auto end_pos = ei.code_writer.append_jump_placeholder(line);

        iterator_value_.update_index(iter_value.index);
        write_idents_->emit(ei);
        iter_value.release();

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

    void AstFor::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFor::visit_children(AstVisitor& vis) {
        iterator_->visit(vis);
        write_idents_->visit(vis);
        body_->visit(vis);
    }
}
