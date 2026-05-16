#include "stdafx.h"
#include "ast_with.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	void AstWith::emit(EmitInfo& ei) {
        assert(ei.stack.empty());        
        ei.states.push();
        value_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::WithInit);
        ei.code_writer.append(line, ident_index_.value_or(std::numeric_limits<std::uint32_t>::max()));
        body_->emit(ei);
        assert(ei.stack.empty());
        ei.code_writer.append(line, ExecuteOp::WithCompleted);
        ei.stack.push();
        ei.stack.pop();
        ei.states.pop();
	}

	void AstWith::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstWith::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        body_->visit(vis);
	}
}