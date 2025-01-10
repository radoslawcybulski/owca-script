#include "stdafx.h"
#include "ast_function.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"
#include "owca_functions.h"

namespace OwcaScript::Internal {
	class ImplExprFunction : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		std::span<AstFunction::CopyFromParent> copy_from_parents;
		std::span<std::string_view> identifier_names;
		std::string_view name;
		ImplStat *body;
		unsigned int param_count;

		void init(std::span<AstFunction::CopyFromParent> copy_from_parents, std::span<std::string_view> identifier_names, std::string_view name, unsigned int param_count, ImplStat *body)
		{
			this->copy_from_parents = copy_from_parents;
			this->identifier_names = identifier_names;
			this->name = name;
			this->body = body;
			this->param_count = param_count;
		}

		OwcaValue execute(OwcaVM &vm) const override
		{
			RuntimeFunction rt;
			rt.body = body;
			rt.code = vm.vm->currently_running_code();
			rt.copy_from_parents = copy_from_parents;
			rt.identifier_names = identifier_names;
			rt.name = name;
			rt.param_count = param_count;

			rt.values_from_parents.reserve(copy_from_parents.size());
			for (auto c : copy_from_parents) {
				rt.values_from_parents.push_back(vm.vm->get_identifier(c.index_in_parent));
			}
			auto of = OwcaFunctions{ vm };
			of.add(std::move(rt));

			return of;
		}
	};


	ImplExpr* AstFunction::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprFunction>(line);
		auto nm = ei.code_buffer.allocate(name_);
		auto c = ei.code_buffer.preallocate_array<CopyFromParent>(copy_from_parents.size());
		for (auto i = 0u; i < copy_from_parents.size(); ++i) {
			c[i] = copy_from_parents[i];
		}
		auto inm = ei.code_buffer.preallocate_array<std::string_view>(identifier_names.size());
		for (auto i = 0u; i < identifier_names.size(); ++i) {
			inm[i] = ei.code_buffer.allocate(identifier_names[i]);
		}
		auto bd = body->emit(ei);
		ret->init(c, inm, nm, (unsigned int)params.size(), bd);
		return ret;
	}

	void AstFunction::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFunction::visit_children(AstVisitor& vis) {
		body->visit(vis);
	}
}