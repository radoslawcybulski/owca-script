#include "stdafx.h"
#include "ast_for.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"
#include "owca_iterator.h"
#include "generator.h"

namespace OwcaScript::Internal {
	// class ImplFor : public ImplStat {
	// public:
    //     using ImplStat::ImplStat;

	// 	#define FIELDS(Q) \
    //         Q(depth, unsigned int) \
    //         Q(loop_ident_index, unsigned int) \
    //         Q(value_indexes, std::span<unsigned int>) \
    //         Q(iterator, ImplExpr*) \
    //         Q(body, ImplStat*)
    
    //     IMPL_DEFINE_STAT(Kind::For)

    //     void execute_statement_impl(OwcaVM vm) const override {
    //         auto counter = (Number)0;
    //         auto iter_value = iterator->execute_expression(vm);
    //         auto iter = [&]() {
    //             if (iter_value.kind() == OwcaValueKind::Iterator) return iter_value.as_iterator(vm);
    //             return VM::get(vm).create_iterator(iter_value);
    //         }();

    //         while(true) {
    //             if (loop_ident_index != std::numeric_limits<unsigned int>::max()) {
    //                 VM::get(vm).set_identifier(loop_ident_index, counter);
    //             }
    //             auto v = iter.next();
    //             if (v.kind() == OwcaValueKind::Completed) break;

    //             if (value_indexes.size() == 1) {
    //                 VM::get(vm).set_identifier(value_indexes[0], v);
    //             }
    //             else {
    //                 auto iter = VM::get(vm).iterate_value(v);
    //                 size_t index = 0;

    //                 while(auto vv = iter.next()) {
    //                     if (index >= value_indexes.size()) {
    //                         VM::get(vm).throw_too_many_elements(value_indexes.size());
    //                     }
    //                     VM::get(vm).set_identifier(value_indexes[index], *vv);
    //                     ++index;
    //                 }
    //                 if (index < value_indexes.size()) {
    //                     VM::get(vm).throw_not_enough_elements(value_indexes.size(), index);
    //                 }
    //             }

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
    //         auto iter_value = iterator->execute_expression(vm);
    //         auto iter = VM::get(vm).create_iterator(iter_value);

    //         while(true) {
    //             if (loop_ident_index != std::numeric_limits<unsigned int>::max()) {
    //                 VM::get(vm).set_identifier(loop_ident_index, counter);
    //             }
    //             auto v = iter.next();
    //             if (v.kind() == OwcaValueKind::Completed) break;
                
    //             if (value_indexes.size() == 1) {
    //                 VM::get(vm).set_identifier(value_indexes[0], v);
    //             }
    //             else {
    //                 auto iter = VM::get(vm).iterate_value(v);
    //                 size_t index = 0;

    //                 while(auto vv = iter.next()) {
    //                     if (index >= value_indexes.size()) {
    //                         VM::get(vm).throw_too_many_elements(value_indexes.size());
    //                     }
    //                     VM::get(vm).set_identifier(value_indexes[index], *vv);
    //                     ++index;
    //                 }
    //                 if (index < value_indexes.size()) {
    //                     VM::get(vm).throw_not_enough_elements(value_indexes.size(), index);
    //                 }
    //             }

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

	void AstFor::emit(EmitInfo& ei) {
        iterator_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::ExprToIterator);
        ei.code_writer.append(line, ExecuteOp::ForInit);
        auto end = ei.code_writer.append_placeholder<std::uint32_t>(line);
        ei.code_writer.append(line, loop_ident_index_.value_or(std::numeric_limits<std::uint32_t>::max()));
        ei.code_writer.append(line, loop_control_depth_);
        auto pos = ei.code_writer.position();
        ei.code_writer.append(line, ExecuteOp::ForCondition);
        assert(value_indexes.size() == 1);
        ei.code_writer.append(line, ExecuteOp::ForNext);
        ei.code_writer.append(line, ExecuteOp::ExprIdentifierWrite);
        ei.code_writer.append(line, value_indexes[0]);
        ei.code_writer.append(line, ExecuteOp::ExprPopAndIgnore);
        body_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::Jump);
        ei.code_writer.append(line, pos);
        ei.code_writer.update_placeholder(end, ei.code_writer.position());
        ei.code_writer.append(line, ExecuteOp::ForCompleted);
	}

	void AstFor::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFor::visit_children(AstVisitor& vis) {
        iterator_->visit(vis);
        body_->visit(vis);
	}
}