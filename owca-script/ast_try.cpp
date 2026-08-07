#include "stdafx.h"
#include "ast_try.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	void AstTry::emit(EmitInfo& ei) {
        auto try_with_index = ei.per_function.add_try_with_block();
        ei.per_function.try_with_all_blocks[try_with_index].begin = ei.code_writer.position();
        body_->emit(ei);
        ei.per_function.try_with_all_blocks[try_with_index].end = ei.code_writer.position();
        ei.per_function.pop_try_with_block(try_with_index);

        ei.per_function.deferred_actions.push_back([=, this, &ei]() {
            ei.per_function.try_with_all_blocks[try_with_index].jump = ei.code_writer.position();

            for(auto & [name, var_index, exprs, stat] : catches_) {
                std::vector<TempInfo> arg_temps;
                for(auto &q : exprs) {
                    arg_temps.push_back(q->emit(ei));
                }
                auto &line = stat->line;
                ei.code_writer.append(line, ExecuteOp::TryCatchType);
                ei.code_writer.append(line, (std::uint32_t)exprs.size());
                ei.code_writer.append(line, var_index);
                for(auto &t : arg_temps) {
                    ei.code_writer.append(line, t.index);
                }
            }
            if (ei.per_function.try_with_all_blocks[try_with_index].next == 0xffffffff) {
                ei.code_writer.append(line, ExecuteOp::TryCatchTypeCompleted);
            }
        });
	}

	void AstTry::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstTry::visit_children(AstVisitor& vis) {
        body_->visit(vis);
        for(auto &c : catches_) {
            for(auto &e : std::get<2>(c)) {
                e->visit(vis);
            }
            std::get<3>(c)->visit(vis);
        }
	}
}