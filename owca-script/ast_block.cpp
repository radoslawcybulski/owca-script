#include "stdafx.h"
#include "ast_block.h"
#include "vm.h"

namespace OwcaScript::Internal {
// 	class ImplBlock : public ImplStat {
// 	public:
// 		using ImplStat::ImplStat;

// #define FIELDS(Q) \
// 		Q(stats, std::span<ImplStat*>)

// 		IMPL_DEFINE_STAT(Kind::Block)

// 		void execute_statement_impl(OwcaVM vm) const override{
// 			for (auto c : stats) {
// 				c->execute_statement(vm);
// 			}
// 		}
// 		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
//  			VM::get(vm).update_execution_line(line);
// 			auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };
// 			for (auto c : stats) {
// 				co_await c->execute_generator_statement(vm, st);
// 			}
// 		}
// 		size_t calculate_generator_allocation_size() const override {
// 			size_t sz = 0;
// 			for(auto c : stats) {
// 				sz = std::max(sz, c->calculate_generator_allocation_size());
// 			}
// 			return sz + calculate_generator_object_size_for_this();
// 		}
// 	};

	void AstBlock::emit(EmitInfo& ei) {
		if (children_.size() == 1)
			return children_[0]->emit(ei);

		for(auto &c : children_) {
			c->emit(ei);
		}
	}

	void AstBlock::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstBlock::visit_children(AstVisitor& vis) {
		for (auto& c : children_) {
			c->visit(vis);
		}
	}
}