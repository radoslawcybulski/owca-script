#include "stdafx.h"
#include "ast_expr_oper_x.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprOperX::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		std::vector<TempInfo> arg_temps;
		for(auto &a : args_) arg_temps.push_back(a->emit(ei));
		switch (kind_) {
		case Kind::Call: ei.code_writer.append(line, ExecuteOp::ExprOperXCall); break;
		case Kind::CreateArray: ei.code_writer.append(line, ExecuteOp::ExprOperXCreateArray); break;
		case Kind::CreateTuple: ei.code_writer.append(line, ExecuteOp::ExprOperXCreateTuple); break;
		case Kind::CreateSet: ei.code_writer.append(line, ExecuteOp::ExprOperXCreateSet); break;
		case Kind::CreateMap: ei.code_writer.append(line, ExecuteOp::ExprOperXCreateMap); break;
		}
		if (!target) target = ei.allocate_temporary();
		ei.code_writer.append(line, target->index);
		ei.code_writer.append(line, (std::uint32_t)args_.size());
		for(auto &a : arg_temps) ei.code_writer.append(line, a.index);
		return std::move(*target);
	}

	void AstExprOperX::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOperX::visit_children(AstVisitor& vis) {
		for(auto &a : args_) a->visit(vis);
	}
}