#include "stdafx.h"
#include "ast_expr_identifier.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	// class ImplExprIdentifier : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#define FIELDS(Q) \
	// 		Q(index, unsigned int) \
	// 		Q(identifier, std::string_view) \
	// 		Q(value_to_write, ImplExpr*) \
	// 		Q(function_write, bool)

	// 	IMPL_DEFINE_EXPR(Kind::Ident)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		if (value_to_write) {
	// 			auto v = value_to_write->execute_expression(vm);
	// 			VM::get(vm).set_identifier(index, v, function_write);
	// 			return v;
	// 		}
	// 		return VM::get(vm).get_identifier(index);
	// 	}
	// };

	void AstExprIdentifier::emit(EmitInfo& ei) {
		if (!value_to_write_) ei.stack.push();
		if (value_to_write_) value_to_write_->emit(ei);
		ei.code_writer.append(line, value_to_write_ ? (function_write_ ? ExecuteOp::ExprIdentifierFunctionWrite : ExecuteOp::ExprIdentifierWrite) : ExecuteOp::ExprIdentifierRead);
		ei.code_writer.append(line, value_to_write_index_);
	}
	void AstExprIdentifier::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprIdentifier::visit_children(AstVisitor& vis) {
		if (value_to_write_)
			value_to_write_->visit(vis);
	}
}