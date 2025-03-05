#ifndef RC_OWCA_SCRIPT_AST_TRY_H
#define RC_OWCA_SCRIPT_AST_TRY_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstTry : public AstStat {
		private:
            std::unique_ptr<AstStat> body;
            std::vector<std::tuple<std::string_view, unsigned int, std::vector<std::unique_ptr<AstExpr>>, std::unique_ptr<AstStat>>> catches;

		public:
            AstTry(Line line, std::unique_ptr<AstStat> body, std::vector<std::tuple<std::string_view, unsigned int, std::vector<std::unique_ptr<AstExpr>>, std::unique_ptr<AstStat>>> catches) : AstStat(line), body(std::move(body)), catches(std::move(catches)) {}
            
            size_t catch_count() const { return catches.size(); }
            auto catch_identifier(size_t index) { return std::get<0>(catches[index]); }
            void update_catch_index(size_t index, unsigned int var_index) { std::get<1>(catches[index]) = var_index; }
			auto catch_line(size_t index) { return std::get<3>(catches[index])->line; }
			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
