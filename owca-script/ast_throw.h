#ifndef RC_OWCA_SCRIPT_AST_THROW_H
#define RC_OWCA_SCRIPT_AST_THROW_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstThrow : public AstStat {
		private:
			std::unique_ptr<AstExpr> value;

		public:
        AstThrow(Line line, std::unique_ptr<AstExpr> value) : AstStat(line), value(std::move(value)) {}

			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
