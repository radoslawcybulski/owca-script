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
            std::string strings_;
			IdentifierIndex index_if_single_string_;
		public:
			AstExprInterpretedString(Line line, std::vector<std::unique_ptr<AstExpr>> evals, std::vector<std::uint32_t> sizes, std::string strings) : AstExpr (line), evals(std::move(evals)), sizes(std::move(sizes)), strings_(std::move(strings)) {
				assert(this->sizes.size() == this->evals.size());
			}

			std::string_view strings() const { return strings_; }
			bool single_string() const { return sizes.empty(); }
			void update_index_if_single_string(IdentifierIndex index) { index_if_single_string_ = index; }

			TempInfo emit(EmitInfo& ei, std::optional<TempInfo> target) override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
