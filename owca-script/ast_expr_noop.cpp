#include "stdafx.h"
#include "ast_expr_noop.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprNoop::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		if (!target) return TempInfo{ ei, index_ };
		ei.write_move(line, target->index, index_);
		return std::move(*target);
	}
	void AstExprNoop::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprNoop::visit_children(AstVisitor& vis) {
	}
}