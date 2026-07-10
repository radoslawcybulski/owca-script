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
			std::uint32_t iterator_index_ = 0;
			std::string identifier_name_;
			std::uint8_t loop_control_depth_;

		public:
            AstFor(Line line, std::uint8_t loop_control_depth, std::unique_ptr<AstExpr> iterator, std::unique_ptr<AstExpr> write_idents, std::unique_ptr<AstStat> body) : AstStat(line), 
				iterator_(std::move(iterator)), write_idents_(std::move(write_idents)), body_(std::move(body)), loop_control_depth_(loop_control_depth) {}

			std::string_view identifier_name() const { return identifier_name_; }
			auto &write_idents() { return *write_idents_; }
			auto loop_control_depth() const { return loop_control_depth_; }
			auto &iterator() { return *iterator_; }
			auto &body() { return *body_; }
			void update_iterator_index(std::uint32_t index) { iterator_index_ = index; }
			void update_identifier_name(std::string name) { identifier_name_ = std::move(name); }

			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
