#ifndef RC_OWCA_SCRIPT_AST_WHILE_H
#define RC_OWCA_SCRIPT_AST_WHILE_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstWhile : public AstStat {
		private:
            std::string_view loop_identifier;
			std::unique_ptr<AstExpr> value;
            std::unique_ptr<AstStat> body;
            unsigned int flow_control_depth;
            std::optional<unsigned int> loop_ident_index;

		public:
			AstWhile(Line line, unsigned int flow_control_depth, std::string_view loop_identifier, std::unique_ptr<AstExpr> value, std::unique_ptr<AstStat> body) : AstStat(line), loop_identifier(loop_identifier), value(std::move(value)), body(std::move(body)), flow_control_depth(flow_control_depth) {}

            auto get_loop_identifier() const { return loop_identifier; }
            void update_loop_ident_index(unsigned int index) {
                loop_ident_index = index;
            }
			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
