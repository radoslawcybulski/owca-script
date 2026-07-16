#ifndef RC_OWCA_SCRIPT_AST_EXPR_NOOP_H
#define RC_OWCA_SCRIPT_AST_EXPR_NOOP_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFunction;

		class AstExprNoop : public AstExpr {
			IdentifierIndex index_;
		public:
			AstExprNoop(Line line) : AstExpr(line) {}

			void update_index(IdentifierIndex index) { index_ = index; }

			TempInfo emit(EmitInfo& ei, std::optional<TempInfo> target) override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
