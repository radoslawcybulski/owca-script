#include "stdafx.h"
#include "ast_expr_compare.h"
#include "vm.h"
#include "owca_vm.h"
#include "owca_iterator.h"
#include "array.h"
#include "string.h"
#include "tuple.h"
#include "dictionary.h"
#include "owca_value.h"
#include <cassert>

namespace OwcaScript::Internal {
	AstBase::TempInfo  AstExprCompare::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		if (!target) target = ei.allocate_temporary();
		std::vector<ExecuteBufferWriter::JumpPlaceholder> jump_placeholders;
		auto first_val = first_->emit(ei);
		for (auto& q : nexts_) {
			auto second_val = std::get<2>(q)->emit(ei);
			switch(std::get<0>(q)) {
			case CompareKind::Eq: ei.code_writer.append(line, ExecuteOp::ExprCompareEq); break;
			case CompareKind::NotEq: ei.code_writer.append(line, ExecuteOp::ExprCompareNotEq); break;
			case CompareKind::LessEq: ei.code_writer.append(line, ExecuteOp::ExprCompareLessEq); break;
			case CompareKind::MoreEq: ei.code_writer.append(line, ExecuteOp::ExprCompareMoreEq); break;
			case CompareKind::Less: ei.code_writer.append(line, ExecuteOp::ExprCompareLess); break;
			case CompareKind::More: ei.code_writer.append(line, ExecuteOp::ExprCompareMore); break;
			case CompareKind::Is: ei.code_writer.append(line, ExecuteOp::ExprCompareIs); break;
			case CompareKind::_Count: assert(false);
			}
			ei.code_writer.append(line, target->index);
			ei.code_writer.append(line, first_val.index);
			ei.code_writer.append(line, second_val.index);
			jump_placeholders.emplace_back(ei.code_writer.append_jump_placeholder(line));
			ei.code_writer.append(line, &q == &nexts_.back());
		}
		auto pos = ei.code_writer.position();
		for (auto& ph : jump_placeholders) {
			ei.code_writer.update_jump_placeholder(ph, (std::int32_t)pos);
		}
		return std::move(*target);
	}
	void AstExprCompare::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprCompare::visit_children(AstVisitor& vis) {
		first_->visit(vis);
		for (auto& q : nexts_) {
			std::get<2>(q)->visit(vis);
		}
	}
}