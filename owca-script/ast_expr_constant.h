#ifndef RC_OWCA_SCRIPT_AST_EXPR_CONSTANT_H
#define RC_OWCA_SCRIPT_AST_EXPR_CONSTANT_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprConstant : public AstExpr {
		private:
			std::variant<OwcaEmpty, OwcaInt, OwcaFloat, OwcaBool, std::string> value;
			
			template <typename ... F> auto visit(F &&...fns) const {
				struct overloaded : F... {
					using F::operator()...;
				};
				return std::visit(overloaded{std::forward<F>(fns)...}, value);
			}	
		public:
			AstExprConstant(Line line, OwcaEmpty value) : AstExpr (line), value(value) {}
			AstExprConstant(Line line, OwcaInt value) : AstExpr (line), value(value) {}
			AstExprConstant(Line line, OwcaFloat value) : AstExpr (line), value(value) {}
			AstExprConstant(Line line, OwcaBool value) : AstExpr (line), value(value) {}
			AstExprConstant(Line line, std::string value) : AstExpr (line), value(std::move(value)) {}

			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
