#include "stdafx.h"
#include "ast_with.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	// class ImplWith : public ImplStat {
	// public:
    //     using ImplStat::ImplStat;

    //     #define FIELDS(Q) \
    //         Q(ident_index, unsigned int) \
    //         Q(value, ImplExpr*) \
    //         Q(body, ImplStat*)

    //     IMPL_DEFINE_STAT(Kind::With)

	// 	void execute_statement_impl(OwcaVM vm) const override {
    //         auto val = value->execute_expression(vm);
    //         auto fnc = VM::get(vm).member(val, "__enter__");
    //         auto val2 = VM::get(vm).execute_call(fnc, {});

    //         if (ident_index != std::numeric_limits<unsigned int>::max()) {
    //             VM::get(vm).set_identifier(ident_index, val2);
    //         }

    //         struct CleanUp {
    //             const ImplWith *self;
    //             OwcaVM &vm;
    //             OwcaValue val;

    //             CleanUp(const ImplWith *self, OwcaVM &vm, OwcaValue val) : self(self), vm(vm), val(val) {}
    //             ~CleanUp() {
    //                 auto fnc = VM::get(vm).member(val, "__exit__");
    //                 VM::get(vm).execute_call(fnc, {});
    //             }
    //         };
    //         auto clean_up = CleanUp{ this, vm, val };
    //         body->execute_statement(vm);
	// 	}
	// 	Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
    //         VM::get(vm).update_execution_line(line);
    //         auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

    //         auto val = value->execute_expression(vm);
    //         auto fnc = VM::get(vm).member(val, "__enter__");
    //         auto val2 = VM::get(vm).execute_call(fnc, {});

    //         if (ident_index != std::numeric_limits<unsigned int>::max()) {
    //             VM::get(vm).set_identifier(ident_index, val2);
    //         }

    //         struct CleanUp {
    //             const ImplWith *self;
    //             OwcaVM &vm;
    //             OwcaValue val;

    //             CleanUp(const ImplWith *self, OwcaVM &vm, OwcaValue val) : self(self), vm(vm), val(val) {}
    //             ~CleanUp() {
    //                 auto fnc = VM::get(vm).member(val, "__exit__");
    //                 VM::get(vm).execute_call(fnc, {});
    //             }
    //         };
    //         auto clean_up = CleanUp{ this, vm, val };
    //         co_await body->execute_generator_statement(vm, st);

    //         co_return;
	// 	}
	// 	size_t calculate_generator_allocation_size() const override {
	// 		return body->calculate_generator_allocation_size() + calculate_generator_object_size_for_this();
	// 	}
	// };

	void AstWith::emit(EmitInfo& ei) {
        assert(ei.stack.empty());        
        ei.states.push();
        value_->emit(ei);
        ei.code_writer.append(line, ExecuteOp::WithInit);
        ei.code_writer.append(line, ident_index_.value_or(std::numeric_limits<std::uint32_t>::max()));
        body_->emit(ei);
        assert(ei.stack.empty());
        ei.code_writer.append(line, ExecuteOp::WithCompleted);
        ei.stack.push();
        ei.stack.pop();
        ei.states.pop();
	}

	void AstWith::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstWith::visit_children(AstVisitor& vis) {
        value_->visit(vis);
        body_->visit(vis);
	}
}