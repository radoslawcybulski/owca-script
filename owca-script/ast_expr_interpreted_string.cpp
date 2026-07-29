#include "stdafx.h"
#include "ast_expr_interpreted_string.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "string.h"
#include "ast_compiler.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprInterpretedString::emit(EmitInfo& ei, std::optional<TempInfo> target) {
        assert(sizes.size() == evals.size());

        if (sizes.empty()) {
            assert(index_if_single_string_);
            if (target) {
                ei.write_move(line, target->index, index_if_single_string_);
            }
            else {
                target = TempInfo{ ei, index_if_single_string_ };
            }
            return std::move(*target);
        }
        else {
            if (!target) target = ei.allocate_temporary();
            std::vector<TempInfo> eval_results;
            for(auto j = 0u; j < evals.size(); ++j) {
                auto val = evals[j]->emit(ei);
                auto dest = ei.allocate_temporary();
                ei.code_writer.append(line, ExecuteOp::ExprToString);
                ei.code_writer.append(line, dest.index);
                ei.code_writer.append(line, val.index);
                eval_results.push_back(std::move(dest));
            }
            ei.code_writer.append(line, ExecuteOp::ExprConstantStringInterpolated);
            ei.code_writer.append(line, target->index);
            ei.code_writer.append(line, strings_);
            ei.code_writer.append(line, (std::uint32_t)sizes.size());
            for(auto j = 0u; j < evals.size(); ++j) {
                ei.code_writer.append(line, eval_results[j].index);
            }
            for(auto j = 0u; j < sizes.size(); ++j) {
                ei.code_writer.append(line, sizes[j]);
            }
            return std::move(*target);
        }
	}
	void AstExprInterpretedString::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprInterpretedString::visit_children(AstVisitor& vis) {
        for(auto &e : evals) {
            e->visit(vis);
        }
	}
}
