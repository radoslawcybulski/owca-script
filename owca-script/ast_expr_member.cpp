#include "stdafx.h"
#include "ast_expr_member.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprMember::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		auto self = value_->emit(ei);
		if (!target) {
			target = ei.allocate_temporary();
		}
		if (value_to_write_) {
			auto val = value_to_write_->emit(ei);
			auto oper = ExecuteOp::ExprMemberWrite;
			if (self_assign_kind_ != SelfAssignKind::None) {
				oper = (ExecuteOp)((std::uint8_t)self_assign_kind_ - 1 + (std::uint8_t)ExecuteOp::ExprOper2MemberFirst);
			}
			ei.code_writer.append(line, oper);
			ei.code_writer.append(line, target->index);
			ei.code_writer.append(line, self.index);
			ei.code_writer.append_identifier(line, member_);
			ei.code_writer.append(line, val.index);
		}
		else {
			ei.code_writer.append(line, ExecuteOp::ExprMemberRead);
			ei.code_writer.append(line, target->index);
			ei.code_writer.append(line, self.index);
			ei.code_writer.append_identifier(line, member_);
		}
		return std::move(*target);
	}
	void AstExprMember::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprMember::visit_children(AstVisitor& vis) {
		value_->visit(vis);
		if (value_to_write_)
			value_to_write_->visit(vis);
	}
}
