#include "stdafx.h"
#include "ast_expr_constant.h"
#include "vm.h"
#include "owca_vm.h"
#include "ast_compiler.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprConstant::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		visit(
			[&](const OwcaEmpty& v) {
				if (!target) target = ei.allocate_temporary();
				ei.code_writer.append(line, ExecuteOp::ExprConstantEmpty);
				ei.code_writer.append(line, target->index);
			},
			[&](const Number& v) {
				auto index = ei.compiler.get_vm().register_number_constant(v);
				if (target) {
					ei.write_move(line, target->index, index);
				}
				else {
					target = TempInfo{ ei, index };
				}
			},
			[&](const bool& v) {
				if (!target) target = ei.allocate_temporary();
				ei.code_writer.append(line, ExecuteOp::ExprConstantBool);
				ei.code_writer.append(line, target->index);
				ei.code_writer.append(line, v);
			},
			[&](const std::string& v) {
				auto index = ei.compiler.get_vm().register_string_constant(v);
				if (target) {
					ei.write_move(line, target->index, index);
				}
				else {
					target = TempInfo{ ei, index };
				}
			}
		);
		return std::move(*target);
	}
	void AstExprConstant::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprConstant::visit_children(AstVisitor& vis) {
	}
}