#ifndef RC_OWCA_SCRIPT_AST_THROW_H
#define RC_OWCA_SCRIPT_AST_THROW_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstThrow : public AstStat {
		private:
			std::unique_ptr<AstExpr> value_;

		public:
        	AstThrow(Line line, std::unique_ptr<AstExpr> value) : AstStat(line), value_(std::move(value)) {}

			auto &value() { return *value_; }

			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
