#ifndef RC_OWCA_SCRIPT_AST_IF_H
#define RC_OWCA_SCRIPT_AST_IF_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstIf : public AstStat {
		private:
			std::unique_ptr<AstExpr> value;
            std::unique_ptr<AstStat> if_true, if_false;

		public:
			AstIf(Line line, std::unique_ptr<AstExpr> value, std::unique_ptr<AstStat> if_true, std::unique_ptr<AstStat> if_false) : AstStat(line), value(std::move(value)), if_true(std::move(if_true)), if_false(std::move(if_false)) {}

			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
