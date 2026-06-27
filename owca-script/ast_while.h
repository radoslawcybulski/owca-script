#ifndef RC_OWCA_SCRIPT_AST_WHILE_H
#define RC_OWCA_SCRIPT_AST_WHILE_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstWhile : public AstStat {
		private:
			std::unique_ptr<AstExpr> value_;
            std::unique_ptr<AstStat> body_;
            std::uint8_t loop_control_depth_;

		public:
			AstWhile(Line line, std::uint8_t loop_control_depth, std::unique_ptr<AstExpr> value, std::unique_ptr<AstStat> body) : AstStat(line), value_(std::move(value)), body_(std::move(body)), loop_control_depth_(loop_control_depth) {}

			auto &value() { return *value_; }
			auto &body() { return *body_; }

			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
