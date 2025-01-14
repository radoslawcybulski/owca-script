#include "stdafx.h"
#include "ast_expr_member.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprMember : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		ImplExpr* value;
		std::string_view identifier;

		void init(ImplExpr* value, std::string_view identifier) {
			this->value = value;
			this->identifier = identifier;
		}
		OwcaValue execute(OwcaVM &vm) const override {
			auto v = value->execute(vm);
			VM::get(vm).throw_missing_member(v.type(), identifier);
			assert(false);
			return {};
		}
	};

	void AstExprMember::calculate_size(CodeBufferSizeCalculator& ei) const
	{
		ei.code_buffer.preallocate<ImplExprMember>(line);
		ei.code_buffer.allocate(member);
		value->calculate_size(ei);
	}
	ImplExpr* AstExprMember::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprMember>(line);
		auto mem = ei.code_buffer.allocate(member);
		auto v = value->emit(ei);
		ret->init(v, mem);
		return ret;
	}
	void AstExprMember::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprMember::visit_children(AstVisitor& vis) {
		value->visit(vis);
	}
}