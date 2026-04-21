#include "stdafx.h"
#include "ast_expr_member.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	// class ImplExprMemberRead : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#define FIELDS(Q) \
	// 		Q(value, ImplExpr*) \
	// 		Q(identifier, std::string_view)

	// 	IMPL_DEFINE_EXPR(Kind::MemberRead)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto v = value->execute_expression(vm);
	// 		return VM::get(vm).member(v, std::string{ identifier });
	// 	}
	// };
	// class ImplExprMemberWrite : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#undef FIELDS
	// 	#define FIELDS(Q) \
	// 		Q(value, ImplExpr*) \
	// 		Q(identifier, std::string_view) \
	// 		Q(value_to_write, ImplExpr*)

	// 	IMPL_DEFINE_EXPR(Kind::MemberWrite)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto vw = value_to_write->execute_expression(vm);
	// 		auto v = value->execute_expression(vm);
	// 		VM::get(vm).member(v, std::string{ identifier }, std::move(vw));
	// 		return {};
	// 	}
	// };

	void AstExprMember::emit(EmitInfo& ei) {
		value_->emit(ei);
		if (value_to_write_) {
			value_to_write_->emit(ei);
			ei.stack.pop();
		}
		ei.code_writer.append(line, value_to_write_ ? ExecuteOp::ExprMemberWrite : ExecuteOp::ExprMemberRead);
		ei.code_writer.append(line, member_);
	}
	void AstExprMember::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprMember::visit_children(AstVisitor& vis) {
		value_->visit(vis);
		if (value_to_write_)
			value_to_write_->visit(vis);
	}
}