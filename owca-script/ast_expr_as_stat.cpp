#include "stdafx.h"
#include "ast_expr_as_stat.h"
#include "vm.h"

namespace OwcaScript::Internal {
	// class ImplExprAsStat : public ImplStat {
	// public:
	// 	using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
	// 		Q(value, ImplExpr*)

	// 	IMPL_DEFINE_STAT(Kind::ExprAsStat)

	// 	void execute_statement_impl(OwcaVM vm) const override {
	// 		value->execute_expression(vm);
	// 	}
	// 	Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
	// 		VM::get(vm).update_execution_line(line);
	// 		auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };
	// 		value->execute_expression(vm);
	// 		co_return;
	// 	}
	// 	size_t calculate_generator_allocation_size() const override {
	// 		return calculate_generator_object_size_for_this();
	// 	}
	// };

	void AstExprAsStat::emit(EmitInfo& ei) {
		child_->emit(ei);
		ei.code_writer.append(line, ExecuteOp::ExprPopAndIgnore);
	}
	void AstExprAsStat::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprAsStat::visit_children(AstVisitor& vis) {
		child_->visit(vis);
	}
}