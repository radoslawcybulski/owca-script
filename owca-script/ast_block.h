#ifndef RC_OWCA_SCRIPT_AST_BLOCK_H
#define RC_OWCA_SCRIPT_AST_BLOCK_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstBlock : public AstStat {
			std::vector<std::unique_ptr<AstStat>> children;

		public:
			AstBlock(Line line, std::vector<std::unique_ptr<AstStat>> children) : AstStat(line), children(std::move(children)) {}

			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
