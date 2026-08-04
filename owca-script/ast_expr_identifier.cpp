#include "stdafx.h"
#include "ast_expr_identifier.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprIdentifier::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		if (function_write_) {
			assert(self_assign_kind_ == SelfAssignKind::None);
			if (!target) target = ei.allocate_temporary();
			assert(value_to_write_);
			auto dest = value_to_write_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::ExprIdentifierFunctionWrite);
			ei.code_writer.append(line, target->index);
			ei.code_writer.append(line, identifier_index_);
			ei.code_writer.append(line, dest.index);
			return std::move(*target);
		}
		else if (value_to_write_) {
			if (self_assign_kind_ == SelfAssignKind::None) {
				if (!target) {
					return value_to_write_->emit(ei, TempInfo{ ei, identifier_index_ });
				}
				auto right = value_to_write_->emit(ei, TempInfo{ ei, identifier_index_ });
				ei.write_move(line, target->index, right.index);
				return std::move(*target);
			}
			auto right = value_to_write_->emit(ei);
			auto oper = (ExecuteOp)((std::uint8_t)self_assign_kind_ - 1 + (std::uint8_t)ExecuteOp::ExprOper2First);
			ei.code_writer.append(line, oper);
			ei.code_writer.append(line, identifier_index_);
			ei.code_writer.append(line, identifier_index_);
			ei.code_writer.append(line, right.index);
			if (!target) {
				target = TempInfo{ ei, identifier_index_ };
			}
			else if (target->index != identifier_index_) {
				ei.write_move(line, target->index, identifier_index_);
			}
			return std::move(*target);
		}
		else {
			if (target && target->index != identifier_index_) {
				ei.write_move(line, target->index, identifier_index_);
			}
			else {
				target = TempInfo{ ei, identifier_index_ };
			}
		}
		return std::move(*target);
	}
	void AstExprIdentifier::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprIdentifier::visit_children(AstVisitor& vis) {
		if (value_to_write_)
			value_to_write_->visit(vis);
	}
}