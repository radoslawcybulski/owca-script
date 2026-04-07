#ifndef RC_OWCA_SCRIPT_AST_EXPR_INTERPRETED_STRING_H
#define RC_OWCA_SCRIPT_AST_EXPR_INTERPRETED_STRING_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFunction;

		class AstExprInterpretedString : public AstExpr {
            std::vector<std::unique_ptr<AstExpr>> evals;
            std::vector<std::uint32_t> sizes;
            std::string strings;
		public:
			AstExprInterpretedString(Line line, std::vector<std::unique_ptr<AstExpr>> evals, std::vector<std::uint32_t> sizes, std::string strings) : AstExpr (line), evals(std::move(evals)), sizes(std::move(sizes)), strings(std::move(strings)) {
				assert(sizes.size() == evals.size() + 1);
			}

			void emit(EmitInfo& ei) override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
