#ifndef RC_OWCA_SCRIPT_AST_FOR_H
#define RC_OWCA_SCRIPT_AST_FOR_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFor : public AstStat {
		private:
            std::string_view loop_identifier;
			std::vector<std::string_view> values;
			std::unique_ptr<AstExpr> iterator;
            std::unique_ptr<AstStat> body;
            unsigned int flow_control_depth;
            std::optional<unsigned int> loop_ident_index;
			std::vector<unsigned int> value_indexes;

		public:
            AstFor(Line line, unsigned int flow_control_depth, std::string_view loop_identifier, std::vector<std::string_view> values, std::unique_ptr<AstExpr> iterator, std::unique_ptr<AstStat> body) : AstStat(line), loop_identifier(loop_identifier), 
				values(std::move(values)), iterator(std::move(iterator)), body(std::move(body)), flow_control_depth(flow_control_depth) {}

            auto get_loop_identifier() const { return loop_identifier; }
			auto get_values() const { return values; }
            void update_loop_ident_index(unsigned int index) {
                loop_ident_index = index;
            }
			void update_value_indexes(std::vector<unsigned int> v) {
				value_indexes = std::move(v);
			}
			ImplStat* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
