#ifndef RC_OWCA_SCRIPT_AST_FUNCTION_H
#define RC_OWCA_SCRIPT_AST_FUNCTION_H

#include "stdafx.h"
#include "ast_base.h"

namespace OwcaScript {
	namespace Internal {
		class AstFunction : public AstExpr {
		public:
			struct CopyFromParent {
				unsigned int index_in_parent;
				unsigned int index_in_child;
			};
		
		private:
			std::string name_;
			std::vector<std::string> params;
			std::vector<CopyFromParent> copy_from_parents;
			std::vector<std::string_view> identifier_names;
			std::unique_ptr<AstStat> body;
			AstFunction* owner_function = nullptr;

		public:
			AstFunction(Line line, std::string name, std::vector<std::string> params, AstFunction* owner_function) : AstExpr(line), name_(std::move(name)), params(std::move(params)), owner_function(owner_function) {}

			auto name() const { return name_; }
			void update_body(std::unique_ptr<AstStat> body) {
				this->body = std::move(body);
			}
			void update_copy_from_parents(std::vector<CopyFromParent> copy_from_parents) {
				this->copy_from_parents = std::move(copy_from_parents);
			}
			void update_identifier_names(std::vector<std::string_view> identifier_names) {
				this->identifier_names = std::move(identifier_names);
			}
			const auto& parameters() const { return params; }
			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
