#ifndef RC_OWCA_SCRIPT_AST_CLASS_H
#define RC_OWCA_SCRIPT_AST_CLASS_H

#include "stdafx.h"
#include "ast_function.h"
#include "owca_class.h"

namespace OwcaScript {
	namespace Internal {
		class AstClass : public AstExpr {
		private:
			std::vector<std::unique_ptr<AstExpr>> base_classes_;
			std::vector<std::unique_ptr<AstFunction>> members_;
			std::vector<std::string> variable_names_;
			std::string name_;
			std::string full_name_;
			bool native_;
			bool all_variable_names_;

		public:
			AstClass(Line line, std::string_view name, std::string full_name, std::vector<std::unique_ptr<AstExpr>> base_classes, std::vector<std::unique_ptr<AstFunction>> members, std::vector<std::string> variable_names, bool all_variable_names, bool native);

			const auto &name() const { return name_; }
			const auto &full_name() const { return full_name_; }
			const auto &base_classes() const { return base_classes_; }
			const auto &members() const { return members_; }
			const auto &variable_names() const { return variable_names_; }
			bool all_variable_names() const { return all_variable_names_; }
			bool is_native() const { return native_; }

			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
