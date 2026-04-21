#include "stdafx.h"
#include "ast_throw.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	// class ImplThrow : public ImplStat {
	// public:
	// 	using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
	// 		Q(value, ImplExpr*)

	// 	IMPL_DEFINE_STAT(Kind::Throw)

	// 	void execute_statement_impl(OwcaVM vm) const override{
    //         auto v = value->execute_expression(vm);
    //         auto oe = v.as_exception(vm);
    //         throw oe;
	// 	}
	// 	Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
	// 		VM::get(vm).update_execution_line(line);
	// 		auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

	// 		execute_statement_impl(vm);
    //         co_return;
	// 	}
	// 	size_t calculate_generator_allocation_size() const override {
	// 		return calculate_generator_object_size_for_this();
	// 	}
	// };

	void AstThrow::emit(EmitInfo& ei) {
		value_->emit(ei);
		ei.stack.pop();
		ei.code_writer.append(line, ExecuteOp::Throw);
	}

	void AstThrow::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstThrow::visit_children(AstVisitor& vis) {
        value_->visit(vis);
	}
}