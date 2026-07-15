#include "stdafx.h"
#include "ast_expr_constant.h"
#include "vm.h"
#include "owca_vm.h"
#include "ast_compiler.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprConstant::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		assert(index_);
		if (target) {
			ei.write_move(line, target->index, index_);
		}
		else {
			target = TempInfo{ ei, index_ };
		}
		return std::move(*target);
	}
	void AstExprConstant::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprConstant::visit_children(AstVisitor& vis) {
	}
}