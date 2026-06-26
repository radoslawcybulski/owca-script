#ifndef RC_OWCA_SCRIPT_AST_FOR_H
#define RC_OWCA_SCRIPT_AST_FOR_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFor : public AstStat {
		private:
            std::string_view loop_identifier_;
			std::unique_ptr<AstExpr> iterator_;
			std::unique_ptr<AstExpr> write_idents_;
            std::unique_ptr<AstStat> body_;
            std::optional<std::uint32_t> loop_ident_index_;
			std::uint8_t loop_control_depth_;

		public:
            AstFor(Line line, std::uint8_t loop_control_depth, std::string_view loop_identifier, std::unique_ptr<AstExpr> iterator, std::unique_ptr<AstExpr> write_idents, std::unique_ptr<AstStat> body) : AstStat(line), loop_identifier_(loop_identifier), 
				iterator_(std::move(iterator)), write_idents_(std::move(write_idents)), body_(std::move(body)), loop_control_depth_(loop_control_depth) {}

			auto &write_idents() { return *write_idents_; }
			auto loop_control_depth() const { return loop_control_depth_; }
			auto &iterator() { return *iterator_; }
			auto &body() { return *body_; }
            auto loop_identifier() const { return loop_identifier_; }
            void update_loop_ident_index(std::uint32_t index) {
                loop_ident_index_ = index;
            }
			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
