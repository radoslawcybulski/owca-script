#ifndef RC_OWCA_SCRIPT_AST_RETURN_H
#define RC_OWCA_SCRIPT_AST_RETURN_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstReturn : public AstStat {
		private:
			std::unique_ptr<AstExpr> value;

		public:
			AstReturn(Line line, std::unique_ptr<AstExpr> value = nullptr) : AstStat(line), value(std::move(value)) {}

			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
