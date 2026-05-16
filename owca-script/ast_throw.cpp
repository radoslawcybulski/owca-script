#include "stdafx.h"
#include "ast_throw.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	void AstThrow::emit(EmitInfo& ei) {
		value_->emit(ei);
		ei.stack.pop();
		ei.code_writer.append(line, ExecuteOp::Throw);
	}

	void AstThrow::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstThrow::visit_children(AstVisitor& vis) {
        value_->visit(vis);
	}
}