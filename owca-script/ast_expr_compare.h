#ifndef RC_OWCA_SCRIPT_AST_EXPR_COMPARE_H
#define RC_OWCA_SCRIPT_AST_EXPR_COMPARE_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		enum class CompareKind {
			Less,
			LessEq,
			More,
			MoreEq,
			Eq,
			NotEq,
			Is
		};
		class AstExprCompare : public AstExpr {
		private:
			std::unique_ptr<AstExpr> first;
			std::vector<std::tuple<CompareKind, Line, std::unique_ptr<AstExpr>>> nexts;

		public:
			AstExprCompare(Line line, std::unique_ptr<AstExpr> first, std::vector<std::tuple<CompareKind, Line, std::unique_ptr<AstExpr>>> nexts) : AstExpr (line), first(std::move(first)), nexts(std::move(nexts)) {}

			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static bool execute_compare(OwcaVM vm, CompareKind kind, OwcaValue left, OwcaValue right);

			static void initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
