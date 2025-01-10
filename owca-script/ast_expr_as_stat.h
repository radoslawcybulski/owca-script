#ifndef RC_OWCA_SCRIPT_AST_EXPR_AS_STAT_H
#define RC_OWCA_SCRIPT_AST_EXPR_AS_STAT_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprAsStat : public AstStat {
			std::unique_ptr<AstExpr> child;

		public:
			AstExprAsStat(Line line, std::unique_ptr<AstExpr> child) : AstStat(line), child(std::move(child)) {}

			ImplStat* emit(EmitInfo& ei) override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
