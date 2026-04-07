#ifndef RC_OWCA_SCRIPT_AST_RETURN_H
#define RC_OWCA_SCRIPT_AST_RETURN_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstReturn : public AstStat {
		private:
			std::unique_ptr<AstExpr> value_;

		public:
			AstReturn(Line line, std::unique_ptr<AstExpr> value = nullptr) : AstStat(line), value_(std::move(value)) {}

			bool has_value() const { return value_ != nullptr; }
			auto &value() { return *value_; }

			ImplStat* emit(EmitInfo& ei) override;
			
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
