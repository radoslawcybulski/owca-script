#ifndef RC_OWCA_SCRIPT_AST_EXPR_COMPARE_H
#define RC_OWCA_SCRIPT_AST_EXPR_COMPARE_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprCompare : public AstExpr {
		public:
			enum class Kind {
				Less,
				LessEq,
				More,
				MoreEq,
				Eq,
				NotEq,
				Is
			};

		private:
			std::unique_ptr<AstExpr> first;
			std::vector<std::tuple<Kind, Line, std::unique_ptr<AstExpr>>> nexts;

		public:
			AstExprCompare(Line line, std::unique_ptr<AstExpr> first, std::vector<std::tuple<Kind, Line, std::unique_ptr<AstExpr>>> nexts) : AstExpr (line), first(std::move(first)), nexts(std::move(nexts)) {}

			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static bool compare_equal(OwcaVM& vm, const OwcaValue&, const OwcaValue&);
		};
	}
}

#endif
