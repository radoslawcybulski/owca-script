#include "stdafx.h"
#include "ast_expr_constant.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	// class ImplExprConstantEmpty : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#define FIELDS(Q) 

	// 	IMPL_DEFINE_EXPR(Kind::ConstantEmpty)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		return OwcaEmpty{};
	// 	}
	// };
	// class ImplExprConstantBool : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#undef FIELDS		
	// 	#define FIELDS(Q) Q(value, bool)

	// 	IMPL_DEFINE_EXPR(Kind::ConstantBool)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		return value;
	// 	}
	// };
	// class ImplExprConstantFloat : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#undef FIELDS
	// 	#define FIELDS(Q) Q(value, Number)

	// 	IMPL_DEFINE_EXPR(Kind::ConstantFloat)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		return value;
	// 	}
	// };
	// class ImplExprConstantString : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#undef FIELDS
	// 	#define FIELDS(Q) Q(value, std::string_view)

	// 	IMPL_DEFINE_EXPR(Kind::ConstantString)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		return vm.create_string(value);
	// 	}
	// };

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
	}
	void AstExprConstant::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprConstant::visit_children(AstVisitor& vis) {
	}
}