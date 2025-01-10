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

		void execute(OwcaVM &vm) const override{
			OwcaValue v;
			if (value) {
				v = value->execute(vm);
			}
			throw FlowControlReturn{ std::move(v) };
		}
	};

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