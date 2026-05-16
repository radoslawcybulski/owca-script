#include "stdafx.h"
#include "ast_class.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "object.h"
#include "runtime_function.h"

namespace OwcaScript::Internal {
	AstClass::AstClass(Line line, std::string_view name, std::string full_name, std::vector<std::unique_ptr<AstExpr>> base_classes, std::vector<std::unique_ptr<AstFunction>> members, std::vector<std::string> variable_names, bool all_variable_names, bool native) : 
		AstExpr(line), base_classes_(std::move(base_classes)), members_(std::move(members)), variable_names_(std::move(variable_names)), name_(std::string{name}), full_name_(std::move(full_name)), all_variable_names_(all_variable_names), native_(native) {
			assert(this->variable_names_.empty() || native_);
		}

	void AstClass::emit(EmitInfo& ei) {
		ei.states.push();
		ei.code_writer.append(line, ExecuteOp::ClassInit);
		ei.code_writer.append(line, name_);
		ei.code_writer.append(line, full_name_);
		for(auto &q : members_) {
			q->emit(ei);
		}
		for(auto &q : base_classes_) {
			q->emit(ei);
		}
		ei.code_writer.append(line, ExecuteOp::ClassCreate);
		ei.code_writer.append(line, native_);
		ei.code_writer.append(line, (std::uint32_t)base_classes_.size());
		ei.code_writer.append(line, (std::uint32_t)members_.size());
		ei.code_writer.append(line, all_variable_names_);
		ei.code_writer.append(line, (std::uint32_t)variable_names_.size());
		for (auto &q : variable_names_)
			ei.code_writer.append(line, q);

		ei.stack.pop(members_.size() + base_classes_.size());
		ei.stack.push();
		ei.states.pop();
	}

	void AstClass::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstClass::visit_children(AstVisitor& vis) {
		for(auto &q : base_classes_)
			q->visit(vis);
		for(auto &q : members_)
			q->visit(vis);
	}
}