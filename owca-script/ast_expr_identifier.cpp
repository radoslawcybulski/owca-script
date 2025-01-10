#include "stdafx.h"
#include "ast_expr_identifier.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprIdentifier : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		std::string_view identifier;
		ImplExpr* value_to_write = nullptr;
		unsigned int index;

		void init(unsigned int index, std::string_view identifier, ImplExpr* value_to_write) {
			this->identifier = identifier;
			this->index = index;
			this->value_to_write = value_to_write;
		}
		OwcaValue execute(OwcaVM &vm) const override {
			if (value_to_write) {
				auto v = value_to_write->execute(vm);
				vm.vm->set_identifier(index, v);
				return v;
			}
			return vm.vm->get_identifier(index);
		}
	};
	ImplExpr* AstExprIdentifier::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprIdentifier>(line);
		auto str = ei.code_buffer.allocate(identifier_);
		assert(index != std::numeric_limits<unsigned int>::max());
		ImplExpr* vw = nullptr;
		if (value_to_write) vw = value_to_write->emit(ei);
		ret->init(index, str, vw);
		return ret;
	}
	void AstExprIdentifier::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprIdentifier::visit_children(AstVisitor& vis) {
	}
}