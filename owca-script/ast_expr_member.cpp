#include "stdafx.h"
#include "ast_expr_member.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	void AstExprMember::emit(EmitInfo& ei) {
		value_->emit(ei);
		if (value_to_write_) {
			value_to_write_->emit(ei);
			ei.stack.pop();
		}
		ei.code_writer.append(line, value_to_write_ ? ExecuteOp::ExprMemberWrite : ExecuteOp::ExprMemberRead);
		ei.code_writer.append(line, member_);
	}
	void AstExprMember::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprMember::visit_children(AstVisitor& vis) {
		value_->visit(vis);
		if (value_to_write_)
			value_to_write_->visit(vis);
	}
}