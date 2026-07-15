#ifndef RC_OWCA_SCRIPT_AST_EXPR_CONSTANT_H
#define RC_OWCA_SCRIPT_AST_EXPR_CONSTANT_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprConstant : public AstExpr {
		private:
			std::variant<OwcaEmpty, Number, bool, std::string> value_;
			IdentifierIndex index_;
		public:
			AstExprConstant(Line line, OwcaEmpty value) : AstExpr (line), value_(value) {}
			AstExprConstant(Line line, Number value) : AstExpr (line), value_(value) {}
			AstExprConstant(Line line, bool value) : AstExpr (line), value_(value) {}
			AstExprConstant(Line line, std::string value) : AstExpr (line), value_(std::move(value)) {}

			TempInfo emit(EmitInfo& ei, std::optional<TempInfo> target) override;

			auto value() const { return value_; }
			void update_index(IdentifierIndex index) { index_ = index; }
			
			template <typename ... F> auto visit_value(F &&...fns) const {
				return visit_variant(value_, std::forward<F>(fns)...);
			}	

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
