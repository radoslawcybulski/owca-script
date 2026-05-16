#include "stdafx.h"
#include "ast_if.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	void AstIf::emit(EmitInfo& ei) {
		value_->emit(ei);
		ei.stack.pop();
		assert(ei.stack.empty());
		ei.code_writer.append(line, ExecuteOp::If);
		if (if_false_) {
			auto else_pos = ei.code_writer.append_jump_placeholder(line);
			if_true_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::Jump);
			auto end_pos = ei.code_writer.append_jump_placeholder(line);
			ei.code_writer.update_jump_placeholder(else_pos, (std::int32_t)ei.code_writer.position());
			if_false_->emit(ei);
			ei.code_writer.update_jump_placeholder(end_pos, (std::int32_t)ei.code_writer.position());
		}
		else {
			auto end_pos = ei.code_writer.append_jump_placeholder(line);
			if_true_->emit(ei);
			ei.code_writer.update_jump_placeholder(end_pos, (std::int32_t)ei.code_writer.position());
		}
	}

	void AstIf::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstIf::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        if_true_->visit(vis);
        if (if_false_) if_false_->visit(vis);
	}
}