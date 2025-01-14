#include "stdafx.h"
#include "ast_expr_as_stat.h"

namespace OwcaScript::Internal {
	class ImplExprAsStat : public ImplStat {
	public:
		using ImplStat::ImplStat;

		ImplExpr* value;

		void init(ImplExpr* value) {
			this->value = value;
		}

		void execute(OwcaVM &vm) const override {
			value->execute(vm);
		}
	};

	void AstExprAsStat::calculate_size(CodeBufferSizeCalculator &ei) const {
		ei.code_buffer.preallocate<ImplExprAsStat>(line);
		child->calculate_size(ei);
	}
	ImplStat* AstExprAsStat::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprAsStat>(line);
		auto val = child->emit(ei);
		ret->init(val);
		return ret;
	}
	void AstExprAsStat::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprAsStat::visit_children(AstVisitor& vis) {
		child->visit(vis);
	}
}