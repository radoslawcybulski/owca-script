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
			std::unique_ptr<AstExpr> left_, right_, third_;
			Kind kind_;

		public:
			AstExprOper2(Line line, Kind kind, std::unique_ptr<AstExpr> left, std::unique_ptr<AstExpr> right, std::unique_ptr<AstExpr> third = nullptr) : AstExpr (line), left_(std::move(left)), right_(std::move(right)), third_(std::move(third)), kind_(kind) {}

			auto kind() const { return kind_; }
			auto &left() { return *left_; }
			auto &right() { return *right_; }
			auto &third() { return *third_; }
			bool has_third() const { return third_ != nullptr; }
			void update_value_to_write(Kind new_kind, std::unique_ptr<AstExpr> third);
			ImplExpr* emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
