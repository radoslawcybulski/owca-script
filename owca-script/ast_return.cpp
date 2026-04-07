#include "stdafx.h"
#include "ast_return.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	// class ImplReturn : public ImplStat {
	// public:
	// 	using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
	// 		Q(value, ImplExpr*)
    
	// 	IMPL_DEFINE_STAT(Kind::Return)

	// 	void execute_statement_impl(OwcaVM vm) const override{
	// 		OwcaValue v;
	// 		if (value) {
	// 			v = value->execute_expression(vm);
	// 		}
	// 		throw FlowControlReturn{ std::move(v) };
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

	void AstReturn::emit(EmitInfo& ei) {
		if (value_) {
			value_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::ReturnValue);
		}
		else {
			ei.code_writer.append(line, ExecuteOp::Return);
		}
	}

	void AstReturn::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstReturn::visit_children(AstVisitor& vis) {
		if (value_)
			value_->visit(vis);
	}
}