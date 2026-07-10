#include "stdafx.h"
#include "ast_for.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"
#include "owca_iterator.h"
#include "generator.h"

namespace OwcaScript::Internal {
    void AstFor::emit(EmitInfo& ei) {
        assert(ei.stack.empty());
        iterator_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::ExprToIterator);
        ei.code_writer.append(line, ExecuteOp::ExprIdentifierWrite);
        ei.code_writer.append(line, iterator_index_);
        ei.code_writer.append(line, ExecuteOp::ExprPopAndIgnore);
        ei.stack.pop();

        auto continue_position = ei.code_writer.position();
        ei.code_writer.append(line, ExecuteOp::ExprIdentifierRead);
        ei.code_writer.append(line, iterator_index_);
        ei.code_writer.append(line, ExecuteOp::ExprIteratorNextAndJumpIfCompleted);
        auto end_pos = ei.code_writer.append_jump_placeholder(line);
        write_idents_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::ExprPopAndIgnore);
        ei.stack.pop();
        assert(ei.stack.empty());
        ei.break_loops.push_back({ .depth=loop_control_depth_ });
        body_->emit(ei);
        auto break_loop = std::move(ei.break_loops.back());
        ei.break_loops.pop_back();
        assert(ei.stack.empty());
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