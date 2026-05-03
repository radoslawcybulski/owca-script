#include "stdafx.h"
#include "ast_expr_oper_1.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	// class ImplExprOper1 : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#define FIELDS(Q) \
	// 		Q(left, ImplExpr*)

	// 	IMPL_DEFINE_EXPR(Kind::BinNeg)
	// };
	// class ImplExprBinNeg : public ImplExprOper1 {
	// public:
	// 	using ImplExprOper1::ImplExprOper1;

	// 	Kind kind() const override { return Kind::BinNeg; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		auto v = l.as_int(vm);
	// 		return ~v;
	// 	}
	// };
	// class ImplExprLogNot : public ImplExprOper1 {
	// public:
	// 	using ImplExprOper1::ImplExprOper1;

	// 	Kind kind() const override { return Kind::LogNot; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		return !l.is_true();
	// 	}
	// };
	// class ImplExprNegate : public ImplExprOper1 {
	// public:
	// 	using ImplExprOper1::ImplExprOper1;

	// 	Kind kind() const override { return Kind::Negate; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		return -l.as_float(vm);
	// 	}
	// };

	// template <typename T> static ImplExpr* make(AstBase::EmitInfo& ei, Line line, const std::unique_ptr<AstExpr>& left)
	// {
	// 	auto ret = ei.code_buffer.preallocate<T>(line);
	// 	auto l = left->emit(ei);
	// 	ret->init(l);
	// 	return ret;
	// }

	void AstExprOper1::emit(EmitInfo& ei) {
		left_->emit(ei);
		switch (kind_) {
		case Kind::BinNeg: ei.code_writer.append(line, ExecuteOp::ExprOper1BinNeg); break;
		case Kind::LogNot: ei.code_writer.append(line, ExecuteOp::ExprOper1LogNot); break;
		case Kind::Negate: ei.code_writer.append(line, ExecuteOp::ExprOper1Negate); break;
		}
	}

	void AstExprOper1::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOper1::visit_children(AstVisitor& vis) {
		left_->visit(vis);
	}
}