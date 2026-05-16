#include "stdafx.h"
#include "ast_block.h"
#include "vm.h"

namespace OwcaScript::Internal {
	void AstBlock::emit(EmitInfo& ei) {
		if (children_.size() == 1)
			return children_[0]->emit(ei);

		for(auto &c : children_) {
			c->emit(ei);
		}
	}

	void AstBlock::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstBlock::visit_children(AstVisitor& vis) {
		for (auto& c : children_) {
			c->visit(vis);
		}
	}
}