#include "stdafx.h"
#include "ast_yield.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	void AstYield::emit(EmitInfo& ei) {
		value_->emit(ei);
		ei.stack.pop();
		ei.code_writer.append(line, ExecuteOp::Yield);
	}

	void AstYield::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstYield::visit_children(AstVisitor& vis) {
		value_->visit(vis);
	}
}