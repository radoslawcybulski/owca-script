#include "stdafx.h"
#include "ast_expr_oper_1.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprOper1::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		auto left_val = left_->emit(ei);
		switch (kind_) {
		case Kind::BinNeg: ei.code_writer.append(line, ExecuteOp::ExprOper1BinNeg); break;
		case Kind::LogNot: ei.code_writer.append(line, ExecuteOp::ExprOper1LogNot); break;
		case Kind::Negate: ei.code_writer.append(line, ExecuteOp::ExprOper1Negate); break;
		}
		if (!target) target = ei.allocate_temporary();
		ei.code_writer.append(line, target->index);
		ei.code_writer.append(line, left_val.index);
		return std::move(*target);
	}

	void AstExprOper1::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOper1::visit_children(AstVisitor& vis) {
		left_->visit(vis);
	}
}