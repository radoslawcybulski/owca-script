#ifndef RC_OWCA_SCRIPT_AST_EXPR_NOOP_H
#define RC_OWCA_SCRIPT_AST_EXPR_NOOP_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFunction;

		class AstExprNoop : public AstExpr {
		public:
			AstExprNoop(Line line) : AstExpr(line) {}

			void emit(EmitInfo& ei) override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
