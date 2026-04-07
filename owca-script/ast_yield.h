#ifndef RC_OWCA_SCRIPT_AST_YIELD_H
#define RC_OWCA_SCRIPT_AST_YIELD_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstYield : public AstStat {
		private:
			std::unique_ptr<AstExpr> value_;

		public:
			AstYield(Line line, std::unique_ptr<AstExpr> value) : AstStat(line), value_(std::move(value)) {}

			auto &value() { return *value_; }

			ImplStat* emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
