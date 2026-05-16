#include "stdafx.h"
#include "ast_expr_interpreted_string.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "string.h"

namespace OwcaScript::Internal {
	void AstExprInterpretedString::emit(EmitInfo& ei) {
        assert(sizes.size() == evals.size());

        if (sizes.empty()) {
            ei.code_writer.append(line, ExecuteOp::ExprConstantString);
            ei.code_writer.append(line, strings);
            ei.stack.push();
        }
        else {
            for(auto j = 0u; j < evals.size(); ++j) {
                evals[j]->emit(ei);
                ei.code_writer.append(line, ExecuteOp::ExprToString);
            }
            ei.stack.pop(evals.size());
            ei.stack.push();
            ei.code_writer.append(line, ExecuteOp::ExprConstantStringInterpolated);
            ei.code_writer.append(line, strings);
            ei.code_writer.append(line, (std::uint32_t)sizes.size());
            for(auto j = 0u; j < sizes.size(); ++j) {
                ei.code_writer.append(line, sizes[j]);
            }
        }
	}
	void AstExprInterpretedString::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprInterpretedString::visit_children(AstVisitor& vis) {
        for(auto &e : evals) {
            e->visit(vis);
        }
	}
}