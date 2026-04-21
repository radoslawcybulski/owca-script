#ifndef RC_OWCA_SCRIPT_AST_FUNCTION_H
#define RC_OWCA_SCRIPT_AST_FUNCTION_H

#include "stdafx.h"
#include "ast_base.h"
#include "owca_vm.h"

namespace OwcaScript {
	namespace Internal {
		class ExecuteBufferWriter;
		class ExecuteBufferReader;

		class AstFunction : public AstExpr {
		public:
			enum class Native : unsigned char { No = 0, Yes = 1 };
			enum class Generator : unsigned char { No = 0, Yes = 1 };
			struct CopyFromParent {
				std::uint32_t index_in_parent;
				std::uint32_t index_in_child;

				CopyFromParent(std::uint32_t index_in_parent, std::uint32_t index_in_child) : index_in_parent(index_in_parent), index_in_child(index_in_child) {}

				bool compare(const CopyFromParent &o) const {
					return index_in_child == o.index_in_child && index_in_parent == o.index_in_parent;
				}
			};
		
		private:
			std::string name_, full_name_;
			std::vector<CopyFromParent> copy_from_parents_;
			std::vector<std::string_view> identifier_names_;
			size_t param_count_;
			std::unique_ptr<AstStat> body_;
			Native native_;
			Generator generator_;
		public:
			AstFunction(Line line, std::string name, std::string full_name, std::vector<std::string_view> identifier_names, size_t param_count, Native native, Generator generator) : AstExpr(line), name_(std::move(name)), full_name_(std::move(full_name)), 
				identifier_names_(std::move(identifier_names)), param_count_(param_count), native_(native), generator_(generator) {}

			const auto &name() const { return name_; }
			const auto &full_name() const { return full_name_; }
			auto &body() { return *body_; }
			void update_body(std::unique_ptr<AstStat> body) {
				this->body_ = std::move(body);
			}
			void update_copy_from_parents(std::vector<CopyFromParent> copy_from_parents) {
				this->copy_from_parents_ = std::move(copy_from_parents);
			}

			void update_identifier_names(std::vector<std::string_view> identifier_names) {
				this->identifier_names_ = std::move(identifier_names);
			}
			bool is_generator() const { return generator_ == Generator::Yes; }
			auto param_count() const { return param_count_; }
			const auto &identifier_names() const { return identifier_names_; }
			void emit(EmitInfo& ei) override;

			void visit(AstVisitor&) override;
			void visit_children(AstVisitor&) override;
		};
	}
}

#endif
