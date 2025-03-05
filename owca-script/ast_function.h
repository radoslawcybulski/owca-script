#ifndef RC_OWCA_SCRIPT_AST_FUNCTION_H
#define RC_OWCA_SCRIPT_AST_FUNCTION_H

#include "stdafx.h"
#include "ast_base.h"
#include "owca_vm.h"

namespace OwcaScript {
	namespace Internal {
		class Serializer;
		class Deserializer;

		class AstFunction : public AstExpr {
		public:
			enum class Native : unsigned char { No = 0, Yes = 1 };
			enum class Generator : unsigned char { No = 0, Yes = 1 };
			struct CopyFromParent {
				unsigned int index_in_parent;
				unsigned int index_in_child;

				void serialize_object(Serializer &) const;
				void deserialize_object(Deserializer &);
				bool compare(const CopyFromParent &o) const {
					return index_in_child == o.index_in_child && index_in_parent == o.index_in_parent;
				}
			};
		
		private:
			std::string name_, full_name_;
			std::vector<std::string> params;
			std::vector<CopyFromParent> copy_from_parents;
			std::vector<std::string_view> identifier_names;
			std::unique_ptr<AstStat> body;
			Native native;
			Generator generator;
		public:
			AstFunction(Line line, std::string name, std::string full_name, std::vector<std::string> params, Native native, Generator generator) : AstExpr(line), name_(std::move(name)), full_name_(std::move(full_name)), params(std::move(params)), 
				native(native), generator(generator) {}

			const auto &name() const { return name_; }
			void update_body(std::unique_ptr<AstStat> body) {
				this->body = std::move(body);
			}
			void update_copy_from_parents(std::vector<CopyFromParent> copy_from_parents) {
				this->copy_from_parents = std::move(copy_from_parents);
			}
			void update_identifier_names(std::vector<std::string_view> identifier_names) {
				this->identifier_names = std::move(identifier_names);
			}
			bool is_generator() const { return generator == Generator::Yes; }
			const auto& parameters() const { return params; }
			ImplExpr* emit(EmitInfo& ei) override;
			void calculate_size(CodeBufferSizeCalculator &) const override;
			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;

			static void initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions);
		};
	}
}

#endif
