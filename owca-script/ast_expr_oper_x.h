#ifndef RC_OWCA_SCRIPT_AST_EXPR_OPER_X_H
#define RC_OWCA_SCRIPT_AST_EXPR_OPER_X_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprOperX : public AstExpr {
		public:
			enum class Kind {
				Call,
				CreateArray,
				CreateSet,
				CreateMap,
			};
		private:
			std::vector<std::unique_ptr<AstExpr>> args;
			Kind kind;

		public:
			AstExprOperX(Line line, Kind kind, std::vector<std::unique_ptr<AstExpr>> args) : AstExpr (line), args(std::move(args)), kind(kind) {}

			ImplExpr* emit(EmitInfo& ei) override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
