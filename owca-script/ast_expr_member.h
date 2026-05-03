#ifndef RC_OWCA_SCRIPT_AST_EXPR_MEMBER_H
#define RC_OWCA_SCRIPT_AST_EXPR_MEMBER_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprMember : public AstExpr {
		private:
			std::string member_;
			std::unique_ptr<AstExpr> value_;
			std::unique_ptr<AstExpr> value_to_write_ = nullptr;

		public:
			AstExprMember(Line line, std::unique_ptr<AstExpr> value, std::string member, std::unique_ptr<AstExpr> value_to_write = nullptr) : AstExpr (line), member_(std::move(member)), value_(std::move(value)), value_to_write_(std::move(value_to_write)) {}

			const auto &member() const { return member_; }
			auto &value() { return *value_; }
			auto &value_to_write() { return *value_to_write_; }
			bool write() const { return value_to_write_ != nullptr; }

			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			void update_value_to_write(std::unique_ptr<AstExpr> v) { value_to_write_ = std::move(v); }
		};
	}
}

#endif
