#include "stdafx.h"
#include "ast_if.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	class ImplIf : public ImplStat {
	public:
		using ImplStat::ImplStat;

		ImplExpr* value;
        ImplStat *if_true, *if_false;

		void init(ImplExpr *value, ImplStat *if_true, ImplStat *if_false) {
			this->value = value;
            this->if_true = if_true;
            this->if_false = if_false;
		}

		void execute_statement_impl(OwcaVM vm) const override{
            auto v = value->execute_expression(vm);
            auto condition = VM::get(vm).calculate_if_true(v);
            if (condition) {
                if_true->execute_statement(vm);
            }
            else if (if_false) {
                if_false->execute_statement(vm);
            }
		}
	};

	void AstIf::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		ei.code_buffer.preallocate<ImplIf>(line);
        value->calculate_size(ei);
        if_true->calculate_size(ei);
        if (if_false) if_false->calculate_size(ei);
	}
	ImplStat* AstIf::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplIf>(line);
		ImplExpr* v = value->emit(ei);
		ImplStat *if_t = this->if_true->emit(ei);
        ImplStat *if_f = nullptr;
        if (if_false) if_f = if_false->emit(ei);
		ret->init(v, if_t, if_f);
		return ret;
	}

	void AstIf::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstIf::visit_children(AstVisitor& vis) {
        value->visit(vis);
        if_true->visit(vis);
        if (if_false) if_false->visit(vis);
	}
}