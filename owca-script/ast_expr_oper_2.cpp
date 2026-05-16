#include "stdafx.h"
#include "ast_expr_oper_2.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "owca_value.h"
#include "dictionary.h"
#include "array.h"
#include "tuple.h"
#include "owca_iterator.h"
#include "string.h"
#include "range.h"

namespace OwcaScript::Internal {
	void AstExprOper2::emit(EmitInfo& ei) {
		if (kind_ == Kind::LogOr) {
			left_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::ExprRetTrueAndJumpIfTrue);
			auto pos = ei.code_writer.append_jump_placeholder(line);
			ei.stack.pop();
			right_->emit(ei);
			ei.code_writer.update_jump_placeholder(pos, ei.code_writer.position());
		}
		else if (kind_ == Kind::LogAnd) {
			left_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::ExprRetFalseAndJumpIfFalse);
			auto pos = ei.code_writer.append_jump_placeholder(line);
			ei.stack.pop();
			right_->emit(ei);
			ei.code_writer.update_jump_placeholder(pos, ei.code_writer.position());
		}
		else if (kind_ == Kind::MakeRange) {
			if (left_) left_->emit(ei);
			if (right_) right_->emit(ei);
			if (third_) third_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::ExprOper2MakeRange);
			ei.code_writer.append(line, (std::uint8_t)((third_ ? 4 : 0) | (right_ ? 2 : 0) | (left_ ? 1 : 0)));
			unsigned int cnt = 0;
			if (left_) ++cnt;
			if (right_) ++cnt;
			if (third_) ++cnt;
			ei.stack.pop(cnt);
			ei.stack.push();
		}
		else {
			left_->emit(ei);
			right_->emit(ei);
			if (third_) third_->emit(ei);
			ei.stack.pop(third_ ? 2 : 1);
			switch (kind_) {
			case Kind::LogOr:
			case Kind::LogAnd:
			case Kind::MakeRange:
				assert(false);
				break;
			case Kind::BinOr: ei.code_writer.append(line, ExecuteOp::ExprOper2BinOr); break;
			case Kind::BinAnd: ei.code_writer.append(line, ExecuteOp::ExprOper2BinAnd); break;
			case Kind::BinXor: ei.code_writer.append(line, ExecuteOp::ExprOper2BinXor); break;
			case Kind::BinLShift: ei.code_writer.append(line, ExecuteOp::ExprOper2BinLShift); break;
			case Kind::BinRShift: ei.code_writer.append(line, ExecuteOp::ExprOper2BinRShift); break;
			case Kind::Add: ei.code_writer.append(line, ExecuteOp::ExprOper2Add); break;
			case Kind::Sub: ei.code_writer.append(line, ExecuteOp::ExprOper2Sub); break;
			case Kind::Mul: ei.code_writer.append(line, ExecuteOp::ExprOper2Mul); break;
			case Kind::Div: ei.code_writer.append(line, ExecuteOp::ExprOper2Div); break;
			case Kind::Mod: ei.code_writer.append(line, ExecuteOp::ExprOper2Mod); break;
			case Kind::IndexRead: ei.code_writer.append(line, ExecuteOp::ExprOper2IndexRead); break;
			case Kind::IndexWrite:
				assert(third_);
				ei.code_writer.append(line, ExecuteOp::ExprOper2IndexWrite);
				break;
			}
		}
	}

	void AstExprOper2::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOper2::visit_children(AstVisitor& vis) {
		if (left_) left_->visit(vis);
		if (right_) right_->visit(vis);
		if (third_) third_->visit(vis);
	}
	void AstExprOper2::update_value_to_write(Kind new_kind, std::unique_ptr<AstExpr> third) {
		kind_ = new_kind;
		third_ = std::move(third);
	}
}