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

	AstBase::TempInfo AstClass::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		if (!target) target = ei.allocate_temporary();
		std::vector<TempInfo> member_temps, base_classes_temps;
		for(auto &q : members_) {
			member_temps.push_back(q->emit(ei, std::nullopt));
		}
		for(auto &q : base_classes_) {
			base_classes_temps.push_back(q->emit(ei, std::nullopt));
		}
		ei.code_writer.append(line, ExecuteOp::ClassCreate);
		ei.code_writer.append(line, target->index);
		ei.code_writer.append(line, name_);
		ei.code_writer.append(line, full_name_);
		ei.code_writer.append(line, native_);
		ei.code_writer.append(line, (std::uint32_t)base_classes_temps.size());
		ei.code_writer.append(line, (std::uint32_t)member_temps.size());
		ei.code_writer.append(line, all_variable_names_);
		if (!all_variable_names_) {
			ei.code_writer.append(line, (std::uint32_t)variable_names_.size());
			for (auto &q : variable_names_) {
				ei.code_writer.append(line, q);
			}
		}

		for (auto &q : base_classes_temps) {
			ei.code_writer.append(line, q.index);
		}
		for (auto &q : member_temps) {
			ei.code_writer.append(line, q.index);
		}
		return std::move(*target);
	}

	void AstClass::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstClass::visit_children(AstVisitor& vis) {
		for(auto &q : base_classes_)
			q->visit(vis);
		for(auto &q : members_)
			q->visit(vis);
	}
}