#ifndef RC_OWCA_SCRIPT_AST_EXPR_IDENTIFIER_H
#define RC_OWCA_SCRIPT_AST_EXPR_IDENTIFIER_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFunction;

		class AstExprIdentifier : public AstExpr {
			std::string_view identifier_;
			std::unique_ptr<AstExpr> value_to_write_ = nullptr;
			std::pair<std::uint32_t, bool> identifier_index_ = { std::numeric_limits<std::uint32_t>::max(), false };
			bool function_write_ = false;

		public:
			AstExprIdentifier(Line line, std::string_view identifier_) : AstExpr (line), identifier_(std::move(identifier_)) {}

			auto identifier() const { return identifier_; }
			auto &value_to_write() const { return *value_to_write_; }
			auto function_write() const { return function_write_; }
			bool write() const { return value_to_write_ != nullptr; }
			void update_identifier_index(std::uint32_t index, bool is_global = false) { identifier_index_ = { index, is_global }; }
			void update_value_to_write(std::unique_ptr<AstExpr> v) { value_to_write_ = std::move(v); }
			void set_function_write() { function_write_ = true; }

			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
