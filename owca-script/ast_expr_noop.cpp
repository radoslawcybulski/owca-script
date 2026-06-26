#include "stdafx.h"
#include "ast_expr_noop.h"

namespace OwcaScript::Internal {
	void AstExprNoop::emit(EmitInfo& ei) {
		ei.stack.push();
	}
	void AstExprNoop::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprNoop::visit_children(AstVisitor& vis) {
	}
}