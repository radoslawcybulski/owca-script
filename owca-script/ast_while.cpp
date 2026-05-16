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
        auto end = ei.code_writer.append_jump_placeholder(line);
        ei.code_writer.append(line, loop_ident_index_.value_or(std::numeric_limits<std::uint32_t>::max()));
        ei.code_writer.append(line, flow_control_depth_);
        auto pos = ei.code_writer.position();
        ei.code_writer.append(line, ExecuteOp::WhileCondition);
        value_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::WhileNext);
        ei.stack.pop();
        assert(ei.stack.empty());
        body_->emit(ei);
        assert(ei.stack.empty());
        ei.code_writer.append(line, ExecuteOp::Jump);
        ei.code_writer.append_jump_position(line, pos);
        ei.code_writer.update_jump_placeholder(end, (std::int32_t)ei.code_writer.position());
        ei.code_writer.append(line, ExecuteOp::WhileCompleted);
        ei.states.pop();
	}

	void AstWhile::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstWhile::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        body_->visit(vis);
	}
}