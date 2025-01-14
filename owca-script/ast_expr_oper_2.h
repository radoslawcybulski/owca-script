#ifndef RC_OWCA_SCRIPT_AST_EXPR_OPER_2_H
#define RC_OWCA_SCRIPT_AST_EXPR_OPER_2_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprOper2 : public AstExpr {
		public:
			enum class Kind {
				LogOr, LogAnd,
				BinOr, BinAnd, BinXor, BinLShift, BinRShift,
				Add, Sub, Mul, Div, Mod,
				IndexRead, IndexWrite,
				MakeRange,
			};

		private:
			std::unique_ptr<AstExpr> left, right, third;
			Kind kind_;

		public:
			AstExprOper2(Line line, Kind kind, std::unique_ptr<AstExpr> left, std::unique_ptr<AstExpr> right, std::unique_ptr<AstExpr> third = nullptr) : AstExpr (line), left(std::move(left)), right(std::move(right)), third(std::move(third)), kind_(kind) {}

			auto kind() const { return kind_; }
			void update_value_to_write(Kind new_kind, std::unique_ptr<AstExpr> third);
			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
