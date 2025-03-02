#include "stdafx.h"
#include "ast_while.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	class ImplWhile : public ImplStat {
	public:
		using ImplStat::ImplStat;

		ImplExpr* value;
        ImplStat* body;
        unsigned int depth;
        unsigned int loop_ident_index;

		void init(unsigned int depth, unsigned int loop_ident_index, ImplExpr *value, ImplStat* body) {
			this->value = value;
            this->body = body;
            this->depth = depth;
            this->loop_ident_index = loop_ident_index;
		}

		void execute_statement_impl(OwcaVM vm) const override {
            auto counter = (OwcaIntInternal)0;

            while(true) {
                if (loop_ident_index != std::numeric_limits<unsigned int>::max()) {
                    VM::get(vm).set_identifier(loop_ident_index, OwcaInt{ counter });
                }
                auto v = value->execute_expression(vm);
                auto condition = VM::get(vm).calculate_if_true(v);
                if (!condition) break;
                try {
                    body->execute_statement(vm);
                }
                catch(FlowControlContinue e) {
                    if (e.depth == depth)
                        continue;
                    throw;
                }
                catch(FlowControlBreak e) {
                    if (e.depth == depth)
                        break;
                    throw;
                }
                ++counter;
            }
		}
		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
            VM::get(vm).update_execution_line(line);
            auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

            auto counter = (OwcaIntInternal)0;

            while(true) {
                if (loop_ident_index != std::numeric_limits<unsigned int>::max()) {
                    VM::get(vm).set_identifier(loop_ident_index, OwcaInt{ counter });
                }
                auto v = value->execute_expression(vm);
                auto condition = VM::get(vm).calculate_if_true(v);
                if (!condition) break;
                try {
                    co_await body->execute_generator_statement(vm, st);
                }
                catch(FlowControlContinue e) {
                    if (e.depth == depth)
                        continue;
                    throw;
                }
                catch(FlowControlBreak e) {
                    if (e.depth == depth)
                        break;
                    throw;
                }
                ++counter;
            }
            co_return;
		}
		size_t calculate_generator_allocation_size() const override {
			return body->calculate_generator_allocation_size() + calculate_generator_object_size_for_this();
		}
	};

	void AstWhile::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		ei.code_buffer.preallocate<ImplWhile>(line);
        value->calculate_size(ei);
        body->calculate_size(ei);
	}
	ImplStat* AstWhile::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplWhile>(line);
        auto v = value->emit(ei);
		auto b = body->emit(ei);
        auto lii = loop_ident_index ? *loop_ident_index : std::numeric_limits<unsigned int>::max();
		ret->init(flow_control_depth, lii, v, b);
		return ret;
	}

	void AstWhile::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstWhile::visit_children(AstVisitor& vis) {
        value->visit(vis);
        body->visit(vis);
	}
}