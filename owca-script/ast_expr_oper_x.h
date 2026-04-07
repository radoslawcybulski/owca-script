#ifndef RC_OWCA_SCRIPT_AST_EXPR_OPER_X_H
#define RC_OWCA_SCRIPT_AST_EXPR_OPER_X_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstExprOperX : public AstExpr {
		public:
			enum class Kind {
				Call,
				CreateArray,
				CreateTuple,
				CreateSet,
				CreateMap,
			};
		private:
			std::vector<std::unique_ptr<AstExpr>> args_;
			Kind kind_;

		public:
			AstExprOperX(Line line, Kind kind, std::vector<std::unique_ptr<AstExpr>> args) : AstExpr (line), args_(std::move(args)), kind_(kind) {}

			auto kind() const { return kind_; }
			const auto &args() const { return args_; }

			ImplExpr* emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
