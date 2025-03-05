#ifndef RC_OWCA_SCRIPT_AST_EXPR_OPER_1_H
#define RC_OWCA_SCRIPT_AST_EXPR_OPER_1_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprOper1 : public AstExpr {
		public:
			enum class Kind {
				LogNot,
				BinNeg,
				Negate,
			};

		private:
			std::unique_ptr<AstExpr> left;
			Kind kind;

		public:
			AstExprOper1(Line line, Kind kind, std::unique_ptr<AstExpr> left) : AstExpr (line), left(std::move(left)), kind(kind) {}

			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
