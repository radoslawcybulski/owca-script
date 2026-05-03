#ifndef RC_OWCA_SCRIPT_AST_WITH_H
#define RC_OWCA_SCRIPT_AST_WITH_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstWith : public AstStat {
		private:
            std::string_view identifier_;
            std::optional<std::uint32_t> ident_index_;
			std::unique_ptr<AstExpr> value_;
            std::unique_ptr<AstStat> body_;

		public:
            AstWith(Line line, std::string_view identifier, std::unique_ptr<AstExpr> value, std::unique_ptr<AstStat> body) : AstStat(line), identifier_(identifier), value_(std::move(value)), body_(std::move(body)) {}

			auto identifier_index() const { return ident_index_; }
			auto &value() { return *value_; }
			auto &body() { return *body_; }
            auto identifier() const { return identifier_; }
            void update_ident_index(std::uint32_t index) {
                ident_index_ = index;
            }
			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
