#include "stdafx.h"
#include "ast_yield.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	// class ImplYield : public ImplStat {
	// public:
	// 	using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
	// 		Q(value, ImplExpr*)

	// 	IMPL_DEFINE_STAT(Kind::Yield)

	// 	void execute_statement_impl(OwcaVM vm) const override {
	// 		assert(false);
	// 	}
	// 	Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
	// 		VM::get(vm).update_execution_line(line);
	// 		auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

	// 		auto v = value->execute_expression(vm);
	// 		co_await Task::Yield{ vm, st, v };
    //         co_return;
	// 	}
	// 	size_t calculate_generator_allocation_size() const override {
	// 		return calculate_generator_object_size_for_this();
	// 	}
	// };

	void AstYield::emit(EmitInfo& ei) {
		value_->emit(ei);
		ei.stack.pop();
		ei.code_writer.append(line, ExecuteOp::Yield);
	}

	void AstYield::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstYield::visit_children(AstVisitor& vis) {
		value_->visit(vis);
	}
}