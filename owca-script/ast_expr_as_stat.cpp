#include "stdafx.h"
#include "ast_expr_as_stat.h"
#include "vm.h"

namespace OwcaScript::Internal {
	void AstExprAsStat::emit(EmitInfo& ei) {
		child_->emit(ei);
		ei.code_writer.append(line, ExecuteOp::ExprPopAndIgnore);
		ei.stack.pop();
		assert(ei.stack.empty());
	}
	void AstExprAsStat::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprAsStat::visit_children(AstVisitor& vis) {
		child_->visit(vis);
	}
}