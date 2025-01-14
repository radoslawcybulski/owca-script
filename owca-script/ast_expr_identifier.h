#ifndef RC_OWCA_SCRIPT_AST_EXPR_IDENTIFIER_H
#define RC_OWCA_SCRIPT_AST_EXPR_IDENTIFIER_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFunction;

		class AstExprIdentifier : public AstExpr {
			std::string_view identifier_;
			std::unique_ptr<AstExpr> value_to_write = nullptr;
			unsigned int index = std::numeric_limits<unsigned int>::max();
			bool function_write = false;

		public:
			AstExprIdentifier(Line line, std::string_view identifier_) : AstExpr (line), identifier_(std::move(identifier_)) {}

			auto identifier() const { return identifier_; }
			bool write() const { return value_to_write != nullptr; }
			void update_index(unsigned int index) { this->index = index; }
			void update_value_to_write(std::unique_ptr<AstExpr> v) { value_to_write = std::move(v); }
			void set_function_write() { function_write = true; }
			bool is_function_write() const { return function_write; }

			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
