#ifndef RC_OWCA_SCRIPT_AST_WHILE_H
#define RC_OWCA_SCRIPT_AST_WHILE_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstWhile : public AstStat {
		private:
            std::string_view loop_identifier_;
			std::unique_ptr<AstExpr> value_;
            std::unique_ptr<AstStat> body_;
            std::uint8_t flow_control_depth_;
            std::optional<unsigned int> loop_ident_index_;

		public:
			AstWhile(Line line, std::uint8_t flow_control_depth, std::string_view loop_identifier, std::unique_ptr<AstExpr> value, std::unique_ptr<AstStat> body) : AstStat(line), loop_identifier_(loop_identifier), value_(std::move(value)), body_(std::move(body)), flow_control_depth_(flow_control_depth) {}

			auto &value() { return *value_; }
			auto &body() { return *body_; }
            auto loop_identifier() const { return loop_identifier_; }
            void update_loop_ident_index(unsigned int index) {
                loop_ident_index_ = index;
            }
			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
