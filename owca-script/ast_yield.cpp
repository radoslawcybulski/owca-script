#include "stdafx.h"
#include "ast_yield.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	void AstYield::emit(EmitInfo& ei) {
		auto res = value_->emit(ei);
		ei.code_writer.append(line, ExecuteOp::Yield);
		ei.code_writer.append(line, res.index);
	}

	void AstYield::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstYield::visit_children(AstVisitor& vis) {
		value_->visit(vis);
	}
}