#ifndef RC_OWCA_SCRIPT_AST_FOR_H
#define RC_OWCA_SCRIPT_AST_FOR_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFor : public AstStat {
		private:
            std::string_view loop_identifier_;
			std::vector<std::string_view> values_;
			std::unique_ptr<AstExpr> iterator_;
            std::unique_ptr<AstStat> body_;
            std::uint32_t flow_control_depth_;
            std::optional<std::uint32_t> loop_ident_index_;
			std::vector<std::uint32_t> value_indexes;

		public:
            AstFor(Line line, std::uint32_t flow_control_depth, std::string_view loop_identifier, std::vector<std::string_view> values, std::unique_ptr<AstExpr> iterator, std::unique_ptr<AstStat> body) : AstStat(line), loop_identifier_(loop_identifier), 
				values_(std::move(values)), iterator_(std::move(iterator)), body_(std::move(body)), flow_control_depth_(flow_control_depth) {}

			const auto &values() const { return values_; }
			auto &iterator() { return *iterator_; }
			auto &body() { return *body_; }
            auto loop_identifier() const { return loop_identifier_; }
            void update_loop_ident_index(std::uint32_t index) {
                loop_ident_index_ = index;
            }
			void update_value_indexes(std::vector<std::uint32_t> v) {
				value_indexes = std::move(v);
			}
			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
