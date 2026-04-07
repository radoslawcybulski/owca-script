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
	// class ImplExprOper2 : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#define FIELDS(Q) \
	// 		Q(left, ImplExpr*) \
	// 		Q(right, ImplExpr*) \
	// 		Q(third, ImplExpr*)

	// 	IMPL_DEFINE_EXPR(Kind::LogOr)

	// 	void init(ImplExpr* left, ImplExpr* right) {
	// 		init(left, right, nullptr);
	// 	}
	// };
	// class ImplExprLogOr : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::LogOr; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		if (l.is_true()) return l;
	// 		return right->execute_expression(vm);
	// 	}
	// };
	// class ImplExprLogAnd : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::LogAnd; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		if (!l.is_true()) return l;
	// 		return right->execute_expression(vm);
	// 	}
	// };
	// class ImplExprBinOr : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::BinOr; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm).as_int(vm);
	// 		auto r = right->execute_expression(vm).as_int(vm);
	// 		return l | r;
	// 	}
	// };
	// class ImplExprBinAnd : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::BinAnd; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm).as_int(vm);
	// 		auto r = right->execute_expression(vm).as_int(vm);
	// 		return l & r;
	// 	}
	// };
	// class ImplExprBinXor : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::BinXor; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm).as_int(vm);
	// 		auto r = right->execute_expression(vm).as_int(vm);
	// 		return l ^ r;
	// 	}
	// };
	// class ImplExprBinLShift : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::BinLShift; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm).as_int(vm);
	// 		auto r = right->execute_expression(vm).as_int(vm);
	// 		return l << r;
	// 	}
	// };
	// class ImplExprBinRShift : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::BinRShift; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm).as_int(vm);
	// 		auto r = right->execute_expression(vm).as_int(vm);
	// 		return l >> r;
	// 	}
	// };
	// class ImplExprMakeRange : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::MakeRange; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left ? left->execute_expression(vm).as_int(vm) : 0;
	// 		auto r = right ? right->execute_expression(vm).as_int(vm) : std::numeric_limits<Number>::max();
	// 		auto s = third ? third->execute_expression(vm).as_int(vm) : 1;
	// 		auto ret = VM::get(vm).allocate<Range>(0);
	// 		ret->from = l;
	// 		ret->to = r;
	// 		ret->step = s;
	// 		if (s == 0) {
	// 			VM::get(vm).range_step_is_zero();
	// 		}
	// 		return OwcaRange{ ret };
	// 	}
	// };
	// class ImplExprAdd : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::Add; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		auto r = right->execute_expression(vm);
	// 		if (l.kind() == OwcaValueKind::Float && r.kind() == OwcaValueKind::Float) {
	// 			return l.as_float(vm) + r.as_float(vm);
	// 		}
	// 		if (l.kind() == OwcaValueKind::String && r.kind() == OwcaValueKind::String) {
	// 			return VM::get(vm).create_string(l, r);
	// 		}
	// 		VM::get(vm).throw_cant_call(std::format("can't execute {} + {}", l.type(), r.type()));
	// 	}
	// };
	// class ImplExprSub : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::Sub; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		auto r = right->execute_expression(vm);
	// 		if (l.kind() == OwcaValueKind::Float && r.kind() == OwcaValueKind::Float) {
	// 			return l.as_float(vm) - r.as_float(vm);
	// 		}
	// 		VM::get(vm).throw_cant_call(std::format("can't execute {} - {}", l.type(), r.type()));
	// 	}
	// };
	// class ImplExprMul : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::Mul; }
	// 	static OwcaValue mul_string(OwcaVM vm, OwcaValue val, Number mul) {
	// 		if (mul < 0)
	// 			Internal::VM::get(vm).throw_invalid_operand_for_mul_string(std::to_string(mul));
	// 		return VM::get(vm).create_string(val, (size_t)mul);
	// 	}
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		auto r = right->execute_expression(vm);
	// 		if (l.kind() == OwcaValueKind::Float && r.kind() == OwcaValueKind::Float) {
	// 			return l.as_float(vm) * r.as_float(vm);
	// 		}
	// 		if (l.kind() == OwcaValueKind::String && r.kind() == OwcaValueKind::Float) {
	// 			return mul_string(vm, l, r.as_int(vm));
	// 		}
	// 		if (l.kind() == OwcaValueKind::Float && r.kind() == OwcaValueKind::String) {
	// 			return mul_string(vm, r, l.as_int(vm));
	// 		}
	// 		VM::get(vm).throw_cant_call(std::format("can't execute {} * {}", l.type(), r.type()));
	// 	}
	// };
	// class ImplExprDiv : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::Div; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		auto r = right->execute_expression(vm);
	// 		if (l.kind() == OwcaValueKind::Float && r.kind() == OwcaValueKind::Float) {
	// 			if (r.as_float(vm) == 0) {
	// 				Internal::VM::get(vm).throw_division_by_zero();
	// 			}
	// 			return l.as_float(vm) / r.as_float(vm);
	// 		}
	// 		VM::get(vm).throw_cant_call(std::format("can't execute {} / {}", l.type(), r.type()));
	// 	}
	// };
	// class ImplExprMod : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	Kind kind() const override { return Kind::Mod; }
	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto l = left->execute_expression(vm);
	// 		auto r = right->execute_expression(vm);
	// 		if (l.kind() == OwcaValueKind::Float && r.kind() == OwcaValueKind::Float) {
	// 			if (r.as_int(vm) == 0) {
	// 				Internal::VM::get(vm).throw_mod_division_by_zero();
	// 		}
				
	// 			return l.as_int(vm) % r.as_int(vm);
	// 		}
	// 		VM::get(vm).throw_cant_call(std::format("can't execute {} % {}", l.type(), r.type()));
	// 	}
	// };
	// static std::tuple<Number, Number, Number> parse_key(OwcaVM vm, OwcaValue v, OwcaValue key, Number size) {
	// 	return key.visit(
	// 		[&](Number o) -> std::tuple<Number, Number, Number> {
	// 			auto v = key.as_int(vm);
	// 			if (v < 0) v += size;
	// 			if (v < 0 || v >= size) {
	// 				Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is out of range for object of size {}", key, size));
	// 			}
	// 			return std::make_tuple(v, v + 1, 0);
	// 		},
	// 		[&](OwcaRange o) -> std::tuple<Number, Number, Number> {
	// 			auto lower = o.lower();
	// 			auto upper = o.upper();
	// 			auto step = o.step();
	// 			if (lower < 0) lower += size;
	// 			if (upper < 0) upper += size;
	// 			if (step > 0) {
	// 				if (lower >= upper) return { 0, 0, 1 };
	// 				if (upper <= 0) return { 0, 0, 1 };
	// 				if (lower >= size) return { 0, 0, 1 };
	// 				if (lower < 0) {
	// 					auto skip = std::max(Number(0), std::floor(-lower / step) - 1);
	// 					lower += skip * step;
	// 				}
	// 				if (step == 1 && lower < 0) {
	// 					lower = 0;
	// 					if (upper < 0) upper = 0;
	// 				}
	// 				if (upper > size) upper = size;
	// 				return std::make_tuple(lower, upper, step);
	// 			}
	// 			else {
	// 				if (lower <= upper) return { 0, 0, 1 };
	// 				if (upper >= size) return { 0, 0, 1 };
	// 				if (lower < 0) return { 0, 0, 1 };
	// 				if (lower > size) {
	// 					auto skip = std::max(Number(0), std::floor((lower - size) / -step) - 1);
	// 					lower -= skip * step;
	// 				}
	// 				if (upper < 0) upper = Number{ -1 };
	// 				return std::make_tuple(lower, upper, step);
	// 			}
	// 		},
	// 		[&](const auto&) -> std::tuple<Number, Number, Number> {
	// 			Internal::VM::get(vm).throw_value_not_indexable(v.type(), key.type());
	// 		}
	// 	);
	// }
	// static size_t verify_key(OwcaVM vm, Number v, size_t size, OwcaValue orig_key, std::string_view name) {
	// 	if (v < 0 || v >= (Number)size)
	// 		Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is out of range for {} of size {}", orig_key, name, size));
	// 	auto v2 = (size_t)v;
	// 	if (v2 != v)
	// 		Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is out of range for {} of size {} - size_t overflows", orig_key, name, size));
	// 	return v2;
	// }
	// static std::pair<size_t, size_t> verify_key(OwcaVM vm, OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name) {
	// 	auto v1 = k.lower();
	// 	auto v2 = k.upper();
	// 	if (v2 <= v1)
	// 		return { 0, 0 };
	// 	if (v1 < 0) v1 = 0;
	// 	if (v2 > (Number)size) v2 = size;
	// 	size_t v3 = (size_t)v1, v4 = (size_t)v2;
	// 	if (v3 != v1 || v2 != v4) {
	// 		Internal::VM::get(vm).throw_index_out_of_range(std::format("index values {} is out of range for array of size {} - size_t overflows", orig_key, size));
	// 	}
	// 	return { v3, v4 };
	// }
	
	// class ImplExprIndexRead : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto v = left->execute_expression(vm);
	// 		auto key = right->execute_expression(vm);

	// 		auto orig_key = key;

	// 		return v.visit(
	// 			[&](const OwcaString& o) -> OwcaValue {
	// 				const auto size = (Number)o.internal_value()->size();
	// 				if (size != o.internal_value()->size()) {
	// 					Internal::VM::get(vm).throw_index_out_of_range(std::format("string size {} is too large for Number size to properly handle indexing", o.internal_value()->size()));
	// 				}
	// 				auto [ lower, upper, step ] = parse_key(vm, key, key, size);
	// 				if (step == 0) return o.substr(lower, lower + 1);
	// 				if (step == 1) return o.substr(lower, upper);
	// 				std::string result;
	// 				result.reserve((size_t)std::abs(std::ceil((upper - lower + step) / step)));
	// 				RangeIterator iter(lower, upper, step);
	// 				for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
	// 					result += o.text()[iter.get()];
	// 				}
	// 				return VM::get(vm).create_string_from_view(result);
	// 			},
	// 			[&](const OwcaArray& o) -> OwcaValue {
	// 				const auto size = (Number)o.internal_value()->values.size();
	// 				if (size != o.internal_value()->values.size()) {
	// 					Internal::VM::get(vm).throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
	// 				}

	// 				auto [ lower, upper, step ] = parse_key(vm, key, key, size);
	// 				if (step == 0) return o[lower];
	// 				if (step == 1) return VM::get(vm).create_array(o.internal_value()->sub_deque(lower, upper));
	// 				std::deque<OwcaValue> result;
	// 				RangeIterator iter(lower, upper, step);
	// 				for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
	// 					result.push_back(o[iter.get()]);
	// 				}
	// 				return VM::get(vm).create_array(std::move(result));
	// 			},
	// 			[&](const OwcaTuple& o) -> OwcaValue {
	// 				const auto size = (Number)o.internal_value()->values.size();
	// 				if (size != o.internal_value()->values.size()) {
	// 					Internal::VM::get(vm).throw_index_out_of_range(std::format("tuple size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
	// 				}

	// 				auto [ lower, upper, step ] = parse_key(vm, key, key, size);
	// 				if (step == 0) return o[lower];
	// 				if (step == 1) return VM::get(vm).create_tuple(o.internal_value()->sub_array(lower, upper));
	// 				std::vector<OwcaValue> result;
	// 				result.reserve((size_t)std::abs(std::ceil((upper - lower + step) / step)));
	// 				RangeIterator iter(lower, upper, step);
	// 				for(RangeIterator iter(lower, upper, step); !iter.done(); iter.next()) {
	// 					result.push_back(o[iter.get()]);
	// 				}
	// 				return VM::get(vm).create_tuple(std::move(result));
	// 			},
	// 			[&](const OwcaMap& o) -> OwcaValue {
	// 				return o.internal_value()->dict.read(key);
	// 			},
	// 			[&](const auto&) -> OwcaValue {
	// 				Internal::VM::get(vm).throw_value_not_indexable(v.type());
	// 			}
	// 		);
	// 	}
	// };
	// class ImplExprIndexWrite : public ImplExprOper2 {
	// public:
	// 	using ImplExprOper2::ImplExprOper2;

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override {
	// 		auto v = left->execute_expression(vm);
	// 		auto key = right->execute_expression(vm);
	// 		auto value = third->execute_expression(vm);

	// 		auto orig_key = key;

	// 		return v.visit(
	// 			[&](const OwcaMap& o) -> OwcaValue {
	// 				o.internal_value()->dict.write(key, std::move(value));
	// 				return {};
	// 			},
	// 			[&](OwcaArray o) -> OwcaValue {
	// 				const auto size = (Number)o.internal_value()->values.size();
	// 				if (size != o.internal_value()->values.size()) {
	// 					Internal::VM::get(vm).throw_index_out_of_range(std::format("array size {} is too large for Number size to properly handle indexing", o.internal_value()->values.size()));
	// 				}
	// 				auto [ lower, upper, step ] = parse_key(vm, key, key, size);
	// 				if (step == 0) {
	// 					o[lower] = value;
	// 					return value;
	// 				}
	// 				if (step != 1) {
	// 					Internal::VM::get(vm).range_step_must_be_one_in_left_side_of_write_assign();
	// 				}

	// 				auto iter = VM::get(vm).create_iterator(value);
	// 				auto write = lower;
	// 				auto &values = o.internal_value()->values;
	// 				std::vector<OwcaValue> temp;

	// 				for(auto val = iter.next(); val.kind() != OwcaValueKind::Completed; val = iter.next()) {
	// 					assert(write <= upper);
	// 					if (write < upper) {
	// 						values[write++] = val;
	// 					}
	// 					else {
	// 						assert(write == upper);
	// 						temp.push_back(val);
	// 					}
	// 				}
	// 				if (write < upper) {
	// 					assert(temp.empty());

	// 					for(size_t i = upper; i < values.size(); ++i) {
	// 						values[write++] = values[i];
	// 					}
	// 					assert(write < values.size());
	// 					values.resize(write);
	// 				}
	// 				else if (!temp.empty()) {
	// 					auto old_size = values.size();
	// 					values.resize(values.size() + temp.size());
	// 					auto new_size = values.size();
	// 					for(size_t i = old_size; i > upper; --i, --new_size) {
	// 						values[new_size] = values[i];
	// 					}
	// 					for(auto q : temp) {
	// 						values[write++] = q;
	// 					}
	// 					assert(write == new_size);
	// 				}
	// 				return {};
	// 			},
	// 			[&](OwcaTuple o) -> OwcaValue {
	// 				Internal::VM::get(vm).throw_readonly("tuple is readonly");
	// 			},
	// 			[&](const auto&) -> OwcaValue {
	// 				Internal::VM::get(vm).throw_value_not_indexable(v.type());
	// 			}
	// 		);
	// 	}
	// };


	// template <typename T> static ImplExpr* make(AstBase::EmitInfo& ei, Line line, const std::unique_ptr<AstExpr>& left, const std::unique_ptr<AstExpr>& right, const std::unique_ptr<AstExpr>& third = nullptr)
	// {
	// 	auto ret = ei.code_buffer.preallocate<T>(line);
	// 	ImplExpr* l = left ? left->emit(ei) : nullptr;
	// 	ImplExpr* r = right ? right->emit(ei) : nullptr;
	// 	ImplExpr* t = third ? third->emit(ei) : nullptr;
	// 	ret->init(l, r, t);
	// 	return ret;

	// }


	void AstExprOper2::emit(EmitInfo& ei) {
		switch (kind_) {
		case Kind::LogOr: ei.code_writer.append(line, ExecuteOp::ExprOper2LogOr); break;
		case Kind::LogAnd: ei.code_writer.append(line, ExecuteOp::ExprOper2LogAnd); break;
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
		case Kind::MakeRange:
			assert(third_);
			ei.code_writer.append(line, ExecuteOp::ExprOper2MakeRange); break;
		case Kind::IndexRead: ei.code_writer.append(line, ExecuteOp::ExprOper2IndexRead); break;
		case Kind::IndexWrite:
			assert(third_);
			ei.code_writer.append(line, ExecuteOp::ExprOper2IndexWrite);
			break;
		}
		left_->emit(ei);
		right_->emit(ei);
		if (third_) third_->emit(ei);
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