#ifndef RC_OWCA_SCRIPT_AST_WITH_H
#define RC_OWCA_SCRIPT_AST_WITH_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstWith : public AstStat {
		private:
            std::string_view identifier;
            std::optional<unsigned int> ident_index;
			std::unique_ptr<AstExpr> value;
            std::unique_ptr<AstStat> body;

		public:
            AstWith(Line line, std::string_view identifier, std::unique_ptr<AstExpr> value, std::unique_ptr<AstStat> body) : AstStat(line), identifier(identifier), value(std::move(value)), body(std::move(body)) {}

            auto get_identifier() const { return identifier; }
            void update_ident_index(unsigned int index) {
                ident_index = index;
            }
			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
