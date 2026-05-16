#include "stdafx.h"
#include "ast_expr_oper_x.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	void AstExprOperX::emit(EmitInfo& ei) {
		for(auto &a : args_) a->emit(ei);
		switch (kind_) {
		case Kind::Call: ei.code_writer.append(line, ExecuteOp::ExprOperXCall); break;
		case Kind::CreateArray: ei.code_writer.append(line, ExecuteOp::ExprOperXCreateArray); break;
		case Kind::CreateTuple: ei.code_writer.append(line, ExecuteOp::ExprOperXCreateTuple); break;
		case Kind::CreateSet: ei.code_writer.append(line, ExecuteOp::ExprOperXCreateSet); break;
		case Kind::CreateMap: ei.code_writer.append(line, ExecuteOp::ExprOperXCreateMap); break;
		}
		ei.code_writer.append(line, (std::uint32_t)args_.size());
		ei.stack.pop(args_.size());
		ei.stack.push();
	}

	void AstExprOperX::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOperX::visit_children(AstVisitor& vis) {
		for(auto &a : args_) a->visit(vis);
	}
}