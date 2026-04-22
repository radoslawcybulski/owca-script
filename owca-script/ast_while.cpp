#include "stdafx.h"
#include "ast_while.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	// class ImplWhile : public ImplStat {
	// public:
    //     using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
    //         Q(depth, unsigned int) \
    //         Q(loop_ident_index, unsigned int) \
    //         Q(value, ImplExpr*) \
    //         Q(body, ImplStat*)
    
    //     IMPL_DEFINE_STAT(Kind::While)

	// 	void execute_statement_impl(OwcaVM vm) const override {
    //         auto counter = (Number)0;

    //         while(true) {
    //             if (loop_ident_index != std::numeric_limits<unsigned int>::max()) {
    //                 VM::get(vm).set_identifier(loop_ident_index, counter);
    //             }
    //             auto v = value->execute_expression(vm);
    //             auto condition = VM::get(vm).calculate_if_true(v);
    //             if (!condition) break;
    //             try {
    //                 body->execute_statement(vm);
    //             }
    //             catch(FlowControlContinue e) {
    //                 if (e.depth == depth)
    //                     continue;
    //                 throw;
    //             }
    //             catch(FlowControlBreak e) {
    //                 if (e.depth == depth)
    //                     break;
    //                 throw;
    //             }
    //             ++counter;
    //         }
	// 	}
	// 	Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
    //         VM::get(vm).update_execution_line(line);
    //         auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

    //         auto counter = (Number)0;

    //         while(true) {
    //             if (loop_ident_index != std::numeric_limits<unsigned int>::max()) {
    //                 VM::get(vm).set_identifier(loop_ident_index, counter);
    //             }
    //             auto v = value->execute_expression(vm);
    //             auto condition = VM::get(vm).calculate_if_true(v);
    //             if (!condition) break;
    //             try {
    //                 co_await body->execute_generator_statement(vm, st);
    //             }
    //             catch(FlowControlContinue e) {
    //                 if (e.depth == depth)
    //                     continue;
    //                 throw;
    //             }
    //             catch(FlowControlBreak e) {
    //                 if (e.depth == depth)
    //                     break;
    //                 throw;
    //             }
    //             ++counter;
    //         }
    //         co_return;
	// 	}
	// 	size_t calculate_generator_allocation_size() const override {
	// 		return body->calculate_generator_allocation_size() + calculate_generator_object_size_for_this();
	// 	}
	// };

	void AstWhile::emit(EmitInfo& ei) {
        assert(ei.stack.empty());
        ei.states.push();
        ei.code_writer.append(line, ExecuteOp::WhileInit);
        auto end = ei.code_writer.append_jump_placeholder(line);
        ei.code_writer.append(line, loop_ident_index_.value_or(std::numeric_limits<std::uint32_t>::max()));
        ei.code_writer.append(line, flow_control_depth_);
        auto pos = ei.code_writer.position();
        ei.code_writer.append(line, ExecuteOp::WhileCondition);
        value_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::WhileNext);
        ei.stack.pop();
        assert(ei.stack.empty());
        body_->emit(ei);
        assert(ei.stack.empty());
        ei.code_writer.append(line, ExecuteOp::Jump);
        ei.code_writer.append_jump_position(line, pos);
        ei.code_writer.update_jump_placeholder(end, (std::int32_t)ei.code_writer.position());
        ei.code_writer.append(line, ExecuteOp::WhileCompleted);
        ei.states.pop();
	}

	void AstWhile::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstWhile::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        body_->visit(vis);
	}
}