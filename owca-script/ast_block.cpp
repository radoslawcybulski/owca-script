#include "stdafx.h"
#include "ast_block.h"

namespace OwcaScript::Internal {
	class ImplBlock : public ImplStat {
	public:
		using ImplStat::ImplStat;

		std::span<ImplStat*> stats;

		void init(std::span<ImplStat*> stats) {
			this->stats = stats;
		}

		void execute_statement_impl(OwcaVM vm) const override{
			for (auto c : stats) {
				c->execute_statement(vm);
			}
		}
	};

	void AstBlock::calculate_size(CodeBufferSizeCalculator &ei) const {
		if (children.size() == 1)
			return children[0]->calculate_size(ei);

		ei.code_buffer.preallocate<ImplBlock>(line);
		if (children.empty()) {
			return;
		}
		ei.code_buffer.preallocate_array<ImplStat*>(children.size());
		for(auto i = 0u; i < children.size(); ++i) {
			children[i]->calculate_size(ei);
		}
	}
	ImplStat* AstBlock::emit(EmitInfo& ei) {
		if (children.size() == 1)
			return children[0]->emit(ei);

		auto ret = ei.code_buffer.preallocate<ImplBlock>(line);
		if (children.empty()) {
			ret->init({});
			return ret;
		}
		auto arr = ei.code_buffer.preallocate_array<ImplStat*>(children.size());
		for(auto i = 0u; i < children.size(); ++i) {
			arr[i] = children[i]->emit(ei);
		}
		ret->init(arr);
		return ret;
	}

	void AstBlock::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstBlock::visit_children(AstVisitor& vis) {
		for (auto& c : children) {
			c->visit(vis);
		}
	}
}