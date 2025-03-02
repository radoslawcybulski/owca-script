#include "stdafx.h"
#include "ast_block.h"
#include "vm.h"

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
		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
 			VM::get(vm).update_execution_line(line);
			auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };
			for (auto c : stats) {
				co_await c->execute_generator_statement(vm, st);
			}
		}
		size_t calculate_generator_allocation_size() const override {
			size_t sz = 0;
			for(auto c : stats) {
				sz = std::max(sz, c->calculate_generator_allocation_size());
			}
			return sz + calculate_generator_object_size_for_this();
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