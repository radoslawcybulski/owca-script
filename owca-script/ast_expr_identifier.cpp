#include "stdafx.h"
#include "ast_expr_identifier.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	void AstExprIdentifier::emit(EmitInfo& ei) {
		if (value_to_write_) {
			value_to_write_->emit(ei);
		}
		else {
			ei.stack.push();
		}
		if (identifier_index_.second) {
			ei.code_writer.append(line, value_to_write_ ? (function_write_ ? ExecuteOp::ExprGlobalFunctionWrite : ExecuteOp::ExprGlobalWrite) : ExecuteOp::ExprGlobalRead);
		}
		else {
			ei.code_writer.append(line, value_to_write_ ? (function_write_ ? ExecuteOp::ExprIdentifierFunctionWrite : ExecuteOp::ExprIdentifierWrite) : ExecuteOp::ExprIdentifierRead);
		}
		ei.code_writer.append(line, identifier_index_.first);
	}
	void AstExprIdentifier::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprIdentifier::visit_children(AstVisitor& vis) {
		if (value_to_write_)
			value_to_write_->visit(vis);
	}
}