#include "stdafx.h"
#include "ast_loop_control.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	// class ImplLoopControlBreak : public ImplStat {
	// public:
	// 	using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
	// 		Q(depth, unsigned int)
    
	// 	IMPL_DEFINE_STAT(Kind::Break)

	// 	void execute_statement_impl(OwcaVM vm) const override{
	// 		throw FlowControlBreak{ depth };
	// 	}
	// 	Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
	// 		VM::get(vm).update_execution_line(line);
	// 		execute_statement_impl(vm);
    //         co_return;
	// 	}
	// 	size_t calculate_generator_allocation_size() const override {
	// 		return calculate_generator_object_size_for_this();
	// 	}
	// };
	// class ImplLoopControlContinue : public ImplStat {
	// public:
	// 	using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
	// 		Q(depth, unsigned int)
    
	// 	IMPL_DEFINE_STAT(Kind::Continue)

	// 	void execute_statement_impl(OwcaVM vm) const override{
	// 		throw FlowControlContinue{ depth };
	// 	}
	// 	Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
	// 		VM::get(vm).update_execution_line(line);
	// 		execute_statement_impl(vm);
    //         co_return;
	// 	}
	// 	size_t calculate_generator_allocation_size() const override {
	// 		return calculate_generator_object_size_for_this();
	// 	}
	// };

	void AstLoopControl::emit(EmitInfo& ei) {
        switch(mode_) {
        case Mode::Break:
			ei.code_writer.append(line, ExecuteOp::LoopControlBreak);
			ei.code_writer.append(line, depth_);
			break;
        case Mode::Continue:
			ei.code_writer.append(line, ExecuteOp::LoopControlContinue);
			ei.code_writer.append(line, depth_);
			break;
        }
	}

	void AstLoopControl::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstLoopControl::visit_children(AstVisitor& vis) {
	}
}