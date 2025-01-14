#ifndef RC_OWCA_SCRIPT_AST_EXPR_MEMBER_H
#define RC_OWCA_SCRIPT_AST_EXPR_MEMBER_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprMember : public AstExpr {
		private:
			std::string member;
			std::unique_ptr<AstExpr> value;
			std::unique_ptr<AstExpr> value_to_write = nullptr;

		public:
			AstExprMember(Line line, std::unique_ptr<AstExpr> value, std::string member, std::unique_ptr<AstExpr> value_to_write = nullptr) : AstExpr (line), member(std::move(member)), value(std::move(value)), value_to_write(std::move(value_to_write)) {}

			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			void update_value_to_write(std::unique_ptr<AstExpr> v) { value_to_write = std::move(v); }
		};
	}
}

#endif
