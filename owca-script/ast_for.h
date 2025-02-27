#ifndef RC_OWCA_SCRIPT_AST_FOR_H
#define RC_OWCA_SCRIPT_AST_FOR_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFor : public AstStat {
		private:
            std::string_view loop_identifier;
			std::string_view value;
			std::unique_ptr<AstExpr> iterator;
            std::unique_ptr<AstStat> body;
            unsigned int flow_control_depth;
            std::optional<unsigned int> loop_ident_index;
			unsigned int value_index;

		public:
            AstFor(Line line, unsigned int flow_control_depth, std::string_view loop_identifier, std::string_view value, std::unique_ptr<AstExpr> iterator, std::unique_ptr<AstStat> body) : AstStat(line), loop_identifier(loop_identifier), 
				value(value), iterator(std::move(iterator)), body(std::move(body)), flow_control_depth(flow_control_depth) {}

            auto get_loop_identifier() const { return loop_identifier; }
			auto get_value() const { return value; }
            void update_loop_ident_index(unsigned int index) {
                loop_ident_index = index;
            }
			void update_value_index(unsigned int v) {
				value_index = v;
			}
			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
