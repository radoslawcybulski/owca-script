#include "stdafx.h"
#include "ast_if.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	// class ImplIf : public ImplStat {
	// public:
	// 	using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
	// 		Q(value, ImplExpr*) \
	// 		Q(if_true, ImplStat*) \
	// 		Q(if_false, ImplStat*)
    
	// 	IMPL_DEFINE_STAT(Kind::If)

	// 	void execute_statement_impl(OwcaVM vm) const override{
    //         auto v = value->execute_expression(vm);
    //         auto condition = VM::get(vm).calculate_if_true(v);
    //         if (condition) {
    //             if_true->execute_statement(vm);
    //         }
    //         else if (if_false) {
    //             if_false->execute_statement(vm);
    //         }
	// 	}
	// 	Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
	// 		VM::get(vm).update_execution_line(line);
	// 		auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

	// 		auto v = value->execute_expression(vm);
    //         auto condition = VM::get(vm).calculate_if_true(v);
    //         if (condition) {
    //             co_await if_true->execute_generator_statement(vm, st);
    //         }
    //         else if (if_false) {
    //             co_await if_false->execute_generator_statement(vm, st);
    //         }
    //         co_return;
	// 	}
	// 	size_t calculate_generator_allocation_size() const override {
	// 		auto sz = std::max(if_true->calculate_generator_allocation_size(), if_false ? if_false->calculate_generator_allocation_size() : (size_t)0);
	// 		return sz + calculate_generator_object_size_for_this();
	// 	}
	// };

	void AstIf::emit(EmitInfo& ei) {
		value_->emit(ei);
		ei.stack.pop();
		assert(ei.stack.empty());
		ei.code_writer.append(line, ExecuteOp::If);
		if (if_false_) {
			auto else_pos = ei.code_writer.append_jump_placeholder(line);
			if_true_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::Jump);
			auto end_pos = ei.code_writer.append_jump_placeholder(line);
			ei.code_writer.update_jump_placeholder(else_pos, (std::int32_t)ei.code_writer.position());
			if_false_->emit(ei);
			ei.code_writer.update_jump_placeholder(end_pos, (std::int32_t)ei.code_writer.position());
		}
		else {
			auto end_pos = ei.code_writer.append_jump_placeholder(line);
			if_true_->emit(ei);
			ei.code_writer.update_jump_placeholder(end_pos, (std::int32_t)ei.code_writer.position());
		}
	}

	void AstIf::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstIf::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        if_true_->visit(vis);
        if (if_false_) if_false_->visit(vis);
	}
}