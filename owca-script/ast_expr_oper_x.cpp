#include "stdafx.h"
#include "ast_expr_oper_x.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "ast_expr_member.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprOperX::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		std::vector<TempInfo> arg_temps;
		auto oper = ExecuteOp::ExprOperXCall;
		std::optional<std::string_view> member;
		if (kind_ == Kind::Call) {
			oper = ExecuteOp::ExprOperXCall;
			if (auto p = dynamic_cast<AstExprMember*>(args_[0].get())) {
				member = p->member();
				arg_temps.push_back(p->value().emit(ei));
				oper = ExecuteOp::ExprOperXCallWithMember;
			}
		}
		else {
			switch (kind_) {
			case Kind::Call: oper = ExecuteOp::ExprOperXCall; break;
			case Kind::CreateArray: oper = ExecuteOp::ExprOperXCreateArray; break;
			case Kind::CreateTuple: oper = ExecuteOp::ExprOperXCreateTuple; break;
			case Kind::CreateSet: oper = ExecuteOp::ExprOperXCreateSet; break;
			case Kind::CreateMap: oper = ExecuteOp::ExprOperXCreateMap; break;
			}
		}
		for(auto i = arg_temps.size(); i < args_.size(); ++i) arg_temps.push_back(args_[i]->emit(ei));
		ei.code_writer.append(line, oper);
		if (!target) target = ei.allocate_temporary();
		ei.code_writer.append(line, target->index);
		if (member) ei.code_writer.append_identifier(line, *member);
		ei.code_writer.append(line, (std::uint32_t)args_.size());
		for(auto &a : arg_temps) ei.code_writer.append(line, a.index);
		return std::move(*target);
	}

	void AstExprOperX::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOperX::visit_children(AstVisitor& vis) {
		for(auto &a : args_) a->visit(vis);
	}
}