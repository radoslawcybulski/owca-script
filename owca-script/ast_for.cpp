#include "stdafx.h"
#include "ast_for.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"
#include "owca_iterator.h"

namespace OwcaScript::Internal {
	class ImplFor : public ImplStat {
	public:
		using ImplStat::ImplStat;

		ImplExpr* iterator;
        ImplStat* body;
        unsigned int depth;
        unsigned int value_index;
        unsigned int loop_ident_index;

		void init(unsigned int depth, unsigned int loop_ident_index, unsigned int value_index, ImplExpr *iterator, ImplStat* body) {
			this->iterator = iterator;
            this->body = body;
            this->depth = depth;
            this->value_index = value_index;
            this->loop_ident_index = loop_ident_index;
		}

		void execute_statement_impl(OwcaVM vm) const override {
            auto counter = (OwcaIntInternal)0;
            auto iter_value = iterator->execute_expression(vm);
            auto iter = VM::get(vm).create_iterator(iter_value);

            while(true) {
                if (loop_ident_index != std::numeric_limits<unsigned int>::max()) {
                    VM::get(vm).set_identifier(loop_ident_index, OwcaInt{ counter });
                }
                auto v = iter->next();
                if (v.kind() == OwcaValueKind::Completed) break;

                VM::get(vm).set_identifier(value_index, v);

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
            auto iter_value = iterator->execute_expression(vm);
            auto iter = VM::get(vm).create_iterator(iter_value);

            while(true) {
                if (loop_ident_index != std::numeric_limits<unsigned int>::max()) {
                    VM::get(vm).set_identifier(loop_ident_index, OwcaInt{ counter });
                }
                auto v = iter->next();
                if (v.kind() == OwcaValueKind::Completed) break;
                
                std::cout << "QWERTY " << v.to_string() << "\n";
                VM::get(vm).set_identifier(value_index, v);

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

	void AstFor::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		ei.code_buffer.preallocate<ImplFor>(line);
        iterator->calculate_size(ei);
        body->calculate_size(ei);
	}
	ImplStat* AstFor::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplFor>(line);
        auto v = iterator->emit(ei);
		auto b = body->emit(ei);
        auto lii = loop_ident_index ? *loop_ident_index : std::numeric_limits<unsigned int>::max();
		ret->init(flow_control_depth, lii, value_index, v, b);
		return ret;
	}

	void AstFor::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFor::visit_children(AstVisitor& vis) {
        iterator->visit(vis);
        body->visit(vis);
	}
}