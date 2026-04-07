#ifndef RC_OWCA_SCRIPT_AST_IF_H
#define RC_OWCA_SCRIPT_AST_IF_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstIf : public AstStat {
		private:
			std::unique_ptr<AstExpr> value_;
            std::unique_ptr<AstStat> if_true_, if_false_;

		public:
			AstIf(Line line, std::unique_ptr<AstExpr> value, std::unique_ptr<AstStat> if_true, std::unique_ptr<AstStat> if_false) : AstStat(line), value_(std::move(value)), if_true_(std::move(if_true)), if_false_(std::move(if_false)) {}

			auto &value() { return *value_; }
			auto &if_true() { return *if_true_; }
			auto &if_false() { return *if_false_; }
			
			ImplStat* emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
