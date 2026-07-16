#ifndef RC_OWCA_SCRIPT_AST_FOR_H
#define RC_OWCA_SCRIPT_AST_FOR_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFor : public AstStat {
		private:
			std::unique_ptr<AstExpr> iterator_;
			std::unique_ptr<AstExpr> write_idents_;
            std::unique_ptr<AstStat> body_;
			AstExprNoop &iterator_value_;
			std::uint8_t loop_control_depth_;

		public:
            AstFor(Line line, std::uint8_t loop_control_depth, std::unique_ptr<AstExpr> iterator, std::unique_ptr<AstExpr> write_idents, AstExprNoop &iterator_value, std::unique_ptr<AstStat> body) : AstStat(line), 
				iterator_(std::move(iterator)), write_idents_(std::move(write_idents)), body_(std::move(body)), iterator_value_(iterator_value), loop_control_depth_(loop_control_depth) {}

			auto &write_idents() { return *write_idents_; }
			auto loop_control_depth() const { return loop_control_depth_; }
			auto &iterator() { return *iterator_; }
			auto &body() { return *body_; }

			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
