#include "stdafx.h"
#include "ast_expr_oper_2.h"

namespace OwcaScript::Internal {
	AstBase::TempInfo AstExprOper2::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		if (!target) target = ei.allocate_temporary();
		if (kind_ == Kind::LogOr) {
			auto temp_val = left_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::ExprRetTrueAndJumpIfTrue);
			ei.code_writer.append(line, target->index);
			ei.code_writer.append(line, temp_val.index);
			auto pos = ei.code_writer.append_jump_placeholder(line);
			temp_val.release();
			target = right_->emit(ei, std::move(*target));
			ei.code_writer.update_jump_placeholder(pos, ei.code_writer.position());
		}
		else if (kind_ == Kind::LogAnd) {
			auto temp_val =left_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::ExprRetFalseAndJumpIfFalse);
			ei.code_writer.append(line, target->index);
			ei.code_writer.append(line, temp_val.index);
			auto pos = ei.code_writer.append_jump_placeholder(line);
			temp_val.release();
			target = right_->emit(ei, std::move(*target));
			ei.code_writer.update_jump_placeholder(pos, ei.code_writer.position());
		}
		else if (kind_ == Kind::MakeRange) {
			TempInfo left_temp{ VariableIndex{} }, right_temp{ VariableIndex{} }, third_temp{ VariableIndex{}};
			if (left_) left_temp = left_->emit(ei);
			if (right_) right_temp = right_->emit(ei);
			if (third_) third_temp = third_->emit(ei);
			ei.code_writer.append(line, ExecuteOp::ExprOper2MakeRange);
			ei.code_writer.append(line, target->index);
			ei.code_writer.append(line, left_temp.index);
			ei.code_writer.append(line, right_temp.index);
			ei.code_writer.append(line, third_temp.index);
		}
		else {
			auto left_val = left_->emit(ei);
			auto right_val = right_->emit(ei);
			TempInfo third_val{ VariableIndex{} };
			if (third_) third_val =third_->emit(ei);

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
			ei.code_writer.append(line, target->index);
			ei.code_writer.append(line, left_val.index);
			ei.code_writer.append(line, right_val.index);
			if (third_) ei.code_writer.append(line, third_val.index);
		}
		return std::move(*target);
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
