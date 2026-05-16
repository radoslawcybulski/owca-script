#include "stdafx.h"
#include "ast_expr_constant.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	void AstExprConstant::emit(EmitInfo& ei) {
		visit(
			[&](const OwcaEmpty& v) {
				ei.code_writer.append(line, ExecuteOp::ExprConstantEmpty);
			},
			[&](const Number& v) {
				ei.code_writer.append(line, ExecuteOp::ExprConstantFloat);
				ei.code_writer.append(line, v);
			},
			[&](const bool& v) {
				ei.code_writer.append(line, ExecuteOp::ExprConstantBool);
				ei.code_writer.append(line, v);
			},
			[&](const std::string& v) {
				ei.code_writer.append(line, ExecuteOp::ExprConstantString);
				ei.code_writer.append(line, v);
			}
		);
		ei.stack.push();
	}
	void AstExprConstant::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprConstant::visit_children(AstVisitor& vis) {
	}
}