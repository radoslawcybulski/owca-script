#include "stdafx.h"
#include "ast_throw.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	class ImplThrow : public ImplStat {
	public:
		using ImplStat::ImplStat;

		ImplExpr* value;

		void init(ImplExpr *value) {
			this->value = value;
		}

		void execute_impl(OwcaVM vm) const override{
            auto v = value->execute(vm);
            auto oe = v.as_exception(vm);
            throw oe;
		}
	};

	void AstThrow::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		ei.code_buffer.preallocate<ImplThrow>(line);
        value->calculate_size(ei);
	}
	ImplStat* AstThrow::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplThrow>(line);
        auto v = value->emit(ei);
		ret->init(v);
		return ret;
	}

	void AstThrow::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstThrow::visit_children(AstVisitor& vis) {
        value->visit(vis);
	}
}