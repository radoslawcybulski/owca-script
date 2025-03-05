#ifndef RC_OWCA_SCRIPT_AST_CLASS_H
#define RC_OWCA_SCRIPT_AST_CLASS_H

#include "stdafx.h"
#include "ast_base.h"
#include "owca_class.h"

namespace OwcaScript {
	namespace Internal {
		class AstClass : public AstExpr {
			std::vector<std::unique_ptr<AstExpr>> base_classes;
			std::vector<std::unique_ptr<AstFunction>> members;
			std::string name_;
			std::string full_name_;
			bool native;

		public:
			AstClass(Line line, std::string_view name, std::string full_name, std::vector<std::unique_ptr<AstExpr>> base_classes, std::vector<std::unique_ptr<AstFunction>> members, bool native) : AstExpr(line), base_classes(std::move(base_classes)), members(std::move(members)), name_(std::string{name}), full_name_(std::move(full_name)), native(native) {}

			const auto &name() const { return name_; }

			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
