#include "stdafx.h"
#include "ast_return.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	class ImplReturn : public ImplStat {
	public:
		using ImplStat::ImplStat;

		ImplExpr* value;

		void init(ImplExpr *value) {
			this->value = value;
		}

		void execute_statement_impl(OwcaVM vm) const override{
			OwcaValue v;
			if (value) {
				v = value->execute_expression(vm);
			}
			throw FlowControlReturn{ std::move(v) };
		}
	};

	void AstReturn::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		ei.code_buffer.preallocate<ImplReturn>(line);
		if (value) {
			value->calculate_size(ei);
		}
	}
	ImplStat* AstReturn::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplReturn>(line);
		ImplExpr* v = nullptr;
		if (value) {
			v = value->emit(ei);
		}
		ret->init(v);
		return ret;
	}

	void AstReturn::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstReturn::visit_children(AstVisitor& vis) {
		if (value)
			value->visit(vis);
	}
}