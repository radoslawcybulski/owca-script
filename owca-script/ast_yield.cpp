#include "stdafx.h"
#include "ast_yield.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	class ImplYield : public ImplStat {
	public:
		using ImplStat::ImplStat;

		ImplExpr* value;

		void init(ImplExpr *value) {
			this->value = value;
		}

		void execute_statement_impl(OwcaVM vm) const override {
			assert(false);
		}
		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
			VM::get(vm).update_execution_line(line);
			auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

			auto v = value->execute_expression(vm);
			co_await Task::Yield{ vm, st, v };
            co_return;
		}
		size_t calculate_generator_allocation_size() const override {
			return calculate_generator_object_size_for_this();
		}
	};

	void AstYield::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		ei.code_buffer.preallocate<ImplYield>(line);
		value->calculate_size(ei);
	}
	ImplStat* AstYield::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplYield>(line);
		auto v = value->emit(ei);
		ret->init(v);
		return ret;
	}

	void AstYield::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstYield::visit_children(AstVisitor& vis) {
		value->visit(vis);
	}
}