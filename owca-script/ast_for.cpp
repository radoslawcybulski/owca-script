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
        ei.states.push();
        iterator_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::ExprToIterator);
        ei.code_writer.append(line, ExecuteOp::ForInit);
        auto end = ei.code_writer.append_jump_placeholder(line);
        ei.code_writer.append(line, loop_ident_index_.value_or(std::numeric_limits<std::uint32_t>::max()));
        ei.code_writer.append(line, loop_control_depth_);
        auto pos = ei.code_writer.position();
        ei.code_writer.append(line, ExecuteOp::ForCondition);
        assert(value_indexes.size() == 1);
        ei.code_writer.append(line, ExecuteOp::ForNext);
        ei.code_writer.append(line, ExecuteOp::ExprIdentifierWrite);
        ei.code_writer.append(line, value_indexes[0]);
        ei.code_writer.append(line, ExecuteOp::ExprPopAndIgnore);
        ei.stack.pop();
        assert(ei.stack.empty());
        body_->emit(ei);
        assert(ei.stack.empty());
        ei.code_writer.append(line, ExecuteOp::Jump);
        ei.code_writer.append_jump_position(line, pos);
        ei.code_writer.update_jump_placeholder(end, (std::int32_t)ei.code_writer.position());
        ei.code_writer.append(line, ExecuteOp::ForCompleted);
        ei.states.pop();
	}

	void AstFor::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFor::visit_children(AstVisitor& vis) {
        iterator_->visit(vis);
        body_->visit(vis);
	}
}