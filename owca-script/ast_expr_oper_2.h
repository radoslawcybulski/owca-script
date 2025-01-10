#ifndef RC_OWCA_SCRIPT_AST_EXPR_OPER_2_H
#define RC_OWCA_SCRIPT_AST_EXPR_OPER_2_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprOper2 : public AstExpr {
		public:
			enum class Kind {
				LogOr, LogAnd,
				BinOr, BinAnd, BinXor, BinLShift, BinRShift,
				Add, Sub, Mul, Div, Mod,
			};

		private:
			std::unique_ptr<AstExpr> left, right;
			Kind kind;

		public:
			AstExprOper2(Line line, Kind kind, std::unique_ptr<AstExpr> left, std::unique_ptr<AstExpr> right) : AstExpr (line), left(std::move(left)), right(std::move(right)), kind(kind) {}

			ImplExpr* emit(EmitInfo& ei) override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
