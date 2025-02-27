#include "stdafx.h"
#include "ast_expr_oper_2.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "owca_value.h"
#include "dictionary.h"
#include "array.h"
#include "owca_iterator.h"
#include "string.h"

namespace OwcaScript::Internal {
	class ImplExprOper2 : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		ImplExpr* left;
		ImplExpr* right;
		ImplExpr* third = nullptr;

		void init(ImplExpr* left, ImplExpr* right, ImplExpr* third = nullptr) {
			this->left = left;
			this->right = right;
			this->third = third;
		}
	};
	class ImplExprLogOr : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
			if (l.is_true()) return l;
			return right->execute_expression(vm);
		}
	};
	class ImplExprLogAnd : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
			if (!l.is_true()) return l;
			return right->execute_expression(vm);
		}
	};
	class ImplExprBinOr : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm).convert_to_int(vm);
			auto r = right->execute_expression(vm).convert_to_int(vm);
			return OwcaInt{ l | r };
		}
	};
	class ImplExprBinAnd : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm).convert_to_int(vm);
			auto r = right->execute_expression(vm).convert_to_int(vm);
			return OwcaInt{ l & r };
		}
	};
	class ImplExprBinXor : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm).convert_to_int(vm);
			auto r = right->execute_expression(vm).convert_to_int(vm);
			return OwcaInt{ l ^ r };
		}
	};
	class ImplExprBinLShift : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm).convert_to_int(vm);
			auto r = right->execute_expression(vm).convert_to_int(vm);
			return OwcaInt{ l << r };
		}
	};
	class ImplExprBinRShift : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm).convert_to_int(vm);
			auto r = right->execute_expression(vm).convert_to_int(vm);
			return OwcaInt{ l >> r };
		}
	};
	class ImplExprMakeRange : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left ? left->execute_expression(vm).convert_to_int(vm) : std::numeric_limits<OwcaIntInternal>::min();
			auto r = right ? right->execute_expression(vm).convert_to_int(vm) : std::numeric_limits<OwcaIntInternal>::max();
			return OwcaRange{ OwcaInt{ l }, OwcaInt{ r } };
		}
	};
	class ImplExprAdd : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
			auto r = right->execute_expression(vm);
			auto [ li, lf ] = l.get_int_or_float();
			auto [ ri, rf ] = r.get_int_or_float();
			if ((li || lf) && (ri || rf)) {
				auto lfloat = li ? (OwcaFloatInternal)li->internal_value() : lf->internal_value();
				auto rfloat = ri ? (OwcaFloatInternal)ri->internal_value() : rf->internal_value();
				auto val = lfloat + rfloat;

				if (li && ri) {
					auto val2 = li->internal_value() + ri->internal_value();
					if (val == val2) return OwcaInt{ val2 };
				}
				return OwcaFloat{ val };
			}
			if (l.kind() == OwcaValueKind::String && r.kind() == OwcaValueKind::String) {
				return VM::get(vm).create_string(l, r);
			}
			VM::get(vm).throw_cant_call(std::format("can't execute {} + {}", l.type(), r.type()));
		}
	};
	class ImplExprSub : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
			auto r = right->execute_expression(vm);
			auto [ li, lf ] = l.get_int_or_float();
			auto [ ri, rf ] = r.get_int_or_float();
			if ((li || lf) && (ri || rf)) {
				auto lfloat = li ? (OwcaFloatInternal)li->internal_value() : lf->internal_value();
				auto rfloat = ri ? (OwcaFloatInternal)ri->internal_value() : rf->internal_value();
				auto val = lfloat - rfloat;

				if (li && ri) {
					auto val2 = li->internal_value() - ri->internal_value();
					if (val == val2) return OwcaInt{ val2 };
				}
				return OwcaFloat{ val };
			}
			VM::get(vm).throw_cant_call(std::format("can't execute {} - {}", l.type(), r.type()));
		}
	};
	class ImplExprMul : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		static OwcaValue mul_string(OwcaVM vm, OwcaValue val, OwcaIntInternal mul) {
			if (mul < 0)
				Internal::VM::get(vm).throw_invalid_operand_for_mul_string(std::to_string(mul));
			return VM::get(vm).create_string(val, (size_t)mul);
		}
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
			auto r = right->execute_expression(vm);
			auto [ li, lf ] = l.get_int_or_float();
			auto [ ri, rf ] = r.get_int_or_float();
			if ((li || lf) && (ri || rf)) {
				auto lfloat = li ? (OwcaFloatInternal)li->internal_value() : lf->internal_value();
				auto rfloat = ri ? (OwcaFloatInternal)ri->internal_value() : rf->internal_value();
				auto val = lfloat * rfloat;

				if (li && ri) {
					auto val2 = li->internal_value() * ri->internal_value();
					if (val == val2) return OwcaInt{ val2 };
				}
				return OwcaFloat{ val };
			}
			if (l.kind() == OwcaValueKind::String && (ri || rf)) {
				auto rr = r.convert_to_int(vm);
				return mul_string(vm, l, rr);
			}
			if ((li || lf) && r.kind() == OwcaValueKind::String) {
				auto ll = l.convert_to_int(vm);
				return mul_string(vm, r, ll);
			}
			VM::get(vm).throw_cant_call(std::format("can't execute {} * {}", l.type(), r.type()));
		}
	};
	class ImplExprDiv : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
			auto r = right->execute_expression(vm);
			auto [ li, lf ] = l.get_int_or_float();
			auto [ ri, rf ] = r.get_int_or_float();
			if ((li || lf) && (ri || rf)) {
				auto lfloat = li ? (OwcaFloatInternal)li->internal_value() : lf->internal_value();
				auto rfloat = ri ? (OwcaFloatInternal)ri->internal_value() : rf->internal_value();
				if (rfloat == 0) {
					Internal::VM::get(vm).throw_division_by_zero();
				}
				auto val = lfloat + rfloat;

				if (li && ri) {
					auto val2 = li->internal_value() + ri->internal_value();
					if (val == val2) return OwcaInt{ val2 };
				}
				return OwcaFloat{ val };
			}
			VM::get(vm).throw_cant_call(std::format("can't execute {} / {}", l.type(), r.type()));
		}
	};
	class ImplExprMod : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm).convert_to_int(vm);
			auto r = right->execute_expression(vm).convert_to_int(vm);
			if (r == 0) {
				VM::get(vm).throw_mod_division_by_zero();
			}
			return OwcaInt{ l % r };
		}
	};
	static void update_key(OwcaVM vm, OwcaValue& key, OwcaIntInternal size) {
		key.visit(
			[&](OwcaFloat o) {
				auto v = key.convert_to_int(vm);
				if (v < 0) v += size;
				key = OwcaInt{ v };
			},
			[&](OwcaInt o) {
				auto v = o.internal_value();
				if (v < 0) v += size;
				key = OwcaInt{ v };
			},
			[&](OwcaRange o) {
				auto [v1, v2] = o.internal_values();
				if (v1 < 0) v1 += size;
				if (v2 < 0) v2 += size;
				key = OwcaRange{ OwcaInt{ v1 }, OwcaInt{ v2 } };
			},
			[&](const auto&) {}
		);
	}
	static size_t verify_key(OwcaVM vm, OwcaInt k, size_t size, OwcaValue orig_key, std::string_view name) {
		auto v = k.internal_value();
		if (v < 0 || v >= (OwcaIntInternal)size)
			Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is out of range for {} of size {}", orig_key, name, size));
		auto v2 = (size_t)v;
		if (v2 != v)
			Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is out of range for {} of size {} - size_t overflows", orig_key, name, size));
		return v2;
	}
	static std::pair<size_t, size_t> verify_key(OwcaVM vm, OwcaRange k, size_t size, OwcaValue orig_key, std::string_view name) {
		auto [v1, v2] = k.internal_values();
		if (v2 <= v1)
			return { 0, 0 };
		if (v1 < 0) v1 = 0;
		if (v2 > (OwcaIntInternal)size) v2 = size;
		size_t v3 = (size_t)v1, v4 = (size_t)v2;
		if (v3 != v1 || v2 != v4) {
			Internal::VM::get(vm).throw_index_out_of_range(std::format("index values {} is out of range for array of size {} - size_t overflows", orig_key, size));
		}
		return { v3, v4 };
	}
	class ImplExprIndexRead : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto v = left->execute_expression(vm);
			auto key = right->execute_expression(vm);

			auto orig_key = key;

			return v.visit(
				[&](const OwcaString& o) -> OwcaValue {
					const auto size = (OwcaIntInternal)o.internal_value()->size();
					if (size != o.internal_value()->size()) {
						Internal::VM::get(vm).throw_index_out_of_range(std::format("string size {} is too large for OwcaIntInternal size to properly handle indexing, maximum value is {}", o.internal_value()->size(), std::numeric_limits<OwcaIntInternal>::max()));
					}
					update_key(vm, key, size);
					return key.visit(
						[&](OwcaInt k) {
							auto v2 = verify_key(vm, k, size, orig_key, "string");
							return o[v2];
						},
						[&](OwcaFloat k) {
							auto z = (OwcaIntInternal)k.internal_value();
							if (z != (OwcaIntInternal)k.internal_value())
								Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is not a valid intenger", orig_key));

							auto v2 = verify_key(vm, OwcaIntInternal{ z }, size, orig_key, "string");
							return o[v2];
						},
						[&](OwcaRange k) {
							auto [v1, v2] = verify_key(vm, k, size, orig_key, "string");
							return o.substr(v1, v2);
						},
						[&](const auto&) -> OwcaValue {
							Internal::VM::get(vm).throw_value_not_indexable(v.type(), key.type());
						}
					);
				},
				[&](const OwcaArray& o) -> OwcaValue {
					const auto size = (OwcaIntInternal)o.internal_value()->values.size();
					if (size != o.internal_value()->values.size()) {
						Internal::VM::get(vm).throw_index_out_of_range(std::format("array size {} is too large for OwcaIntInternal size to properly handle indexing, maximum value is {}", o.internal_value()->values.size(), std::numeric_limits<OwcaIntInternal>::max()));
					}
					update_key(vm, key, size);
					return key.visit(
						[&](OwcaInt k) -> OwcaValue {
							auto v2 = verify_key(vm, k, size, orig_key, "array");
							return o[v2];
						},
						[&](OwcaFloat k) {
							auto z = (OwcaIntInternal)k.internal_value();
							if (z != (OwcaIntInternal)k.internal_value())
								Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is not a valid intenger", orig_key));

							auto v2 = verify_key(vm, OwcaIntInternal{ z }, size, orig_key, "array");
							return o[v2];
						},
						[&](OwcaRange k) -> OwcaValue {
							auto [v1, v2] = verify_key(vm, k, size, orig_key, "array");
							return VM::get(vm).create_array(o.internal_value()->sub_array(v1, v2));
						},
						[&](const auto&) -> OwcaValue {
							Internal::VM::get(vm).throw_wrong_type(std::format("can't index with {} value", key.type()));
						}
					);
				},
				[&](const OwcaTuple& o) -> OwcaValue {
					const auto size = (OwcaIntInternal)o.internal_value()->values.size();
					if (size != o.internal_value()->values.size()) {
						Internal::VM::get(vm).throw_index_out_of_range(std::format("tuple size {} is too large for OwcaIntInternal size to properly handle indexing, maximum value is {}", o.internal_value()->values.size(), std::numeric_limits<OwcaIntInternal>::max()));
					}
					update_key(vm, key, size);
					return key.visit(
						[&](OwcaInt k) -> OwcaValue {
							auto v2 = verify_key(vm, k, size, orig_key, "tuple");
							return o[v2];
						},
						[&](OwcaFloat k) {
							auto z = (OwcaIntInternal)k.internal_value();
							if (z != (OwcaIntInternal)k.internal_value())
								Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is not a valid intenger", orig_key));

							auto v2 = verify_key(vm, OwcaIntInternal{ z }, size, orig_key, "tuple");
							return o[v2];
						},
						[&](OwcaRange k) -> OwcaValue {
							auto [v1, v2] = verify_key(vm, k, size, orig_key, "tuple");
							return VM::get(vm).create_array(o.internal_value()->sub_array(v1, v2));
						},
						[&](const auto&) -> OwcaValue {
							Internal::VM::get(vm).throw_wrong_type(std::format("can't index with {} value", key.type()));
						}
					);
				},
				[&](const OwcaMap& o) -> OwcaValue {
					return o.internal_value()->dict.read(key);
				},
				[&](const auto&) -> OwcaValue {
					Internal::VM::get(vm).throw_value_not_indexable(v.type());
				}
			);
		}
	};
	class ImplExprIndexWrite : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto v = left->execute_expression(vm);
			auto key = right->execute_expression(vm);
			auto value = third->execute_expression(vm);

			auto orig_key = key;

			return v.visit(
				[&](const OwcaMap& o) -> OwcaValue {
					o.internal_value()->dict.write(key, std::move(value));
					return {};
				},
				[&](OwcaArray o) -> OwcaValue {
					const auto size = (OwcaIntInternal)o.internal_value()->values.size();
					if (size != o.internal_value()->values.size()) {
						Internal::VM::get(vm).throw_index_out_of_range(std::format("array size {} is too large for OwcaIntInternal size to properly handle indexing, maximum value is {}", o.internal_value()->values.size(), std::numeric_limits<OwcaIntInternal>::max()));
					}
					update_key(vm, key, size);
					return key.visit(
						[&](OwcaInt k) -> OwcaValue {
							auto v2 = verify_key(vm, k, size, orig_key, "array");
							o[v2] = std::move(value);
							return o[v2];
						},
						[&](OwcaRange k) -> OwcaValue {
							auto [v1, v2] = verify_key(vm, k, size, orig_key, "array");
							auto iter = VM::get(vm).create_iterator(value);
							auto iter_size = iter->remaining_size();
							if (v2 - v1 != iter_size) {
								auto old = std::move(o.internal_value()->values);
								o.internal_value()->values.resize(old.size() - (v2 - v1) + iter_size);
								for(size_t i = 0u; i < v1; ++i) {
									o.internal_value()->values[i] = std::move(old[i]);
								}
								for(size_t i = v2; i < old.size(); ++i) {
									o.internal_value()->values[i - (v2 - v1) + iter_size] = std::move(old[i]);
								}
							}
							for(size_t i = 0u; i < iter_size; ++i) {
								auto v = iter->get();
								assert(v);
								o.internal_value()->values[v1 +  i] = std::move(*v);
								iter->next();
							}
							return {};
						},
						[&](const auto&) -> OwcaValue {
							Internal::VM::get(vm).throw_wrong_type(std::format("can't index with {} value", key.type()));
						}
					);
				},
				[&](OwcaTuple o) -> OwcaValue {
					Internal::VM::get(vm).throw_readonly("tuple is readonly");
				},
				[&](const auto&) -> OwcaValue {
					Internal::VM::get(vm).throw_value_not_indexable(v.type());
				}
			);
		}
	};


	template <typename T> static ImplExpr* make(AstBase::EmitInfo& ei, Line line, const std::unique_ptr<AstExpr>& left, const std::unique_ptr<AstExpr>& right, const std::unique_ptr<AstExpr>& third = nullptr)
	{
		auto ret = ei.code_buffer.preallocate<T>(line);
		ImplExpr* l = left ? left->emit(ei) : nullptr;
		ImplExpr* r = right ? right->emit(ei) : nullptr;
		ImplExpr* t = third ? third->emit(ei) : nullptr;
		ret->init(l, r, t);
		return ret;

	}
	void AstExprOper2::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		switch (kind_) {
		case Kind::LogOr: ei.code_buffer.preallocate<ImplExprLogOr>(line); break;
		case Kind::LogAnd: ei.code_buffer.preallocate<ImplExprLogAnd>(line); break;
		case Kind::BinOr: ei.code_buffer.preallocate<ImplExprBinOr>(line); break;
		case Kind::BinAnd: ei.code_buffer.preallocate<ImplExprBinAnd>(line); break;
		case Kind::BinXor: ei.code_buffer.preallocate<ImplExprBinXor>(line); break;
		case Kind::BinLShift: ei.code_buffer.preallocate<ImplExprBinLShift>(line); break;
		case Kind::BinRShift: ei.code_buffer.preallocate<ImplExprBinRShift>(line); break;
		case Kind::Add: ei.code_buffer.preallocate<ImplExprAdd>(line); break;
		case Kind::Sub: ei.code_buffer.preallocate<ImplExprSub>(line); break;
		case Kind::Mul: ei.code_buffer.preallocate<ImplExprMul>(line); break;
		case Kind::Div: ei.code_buffer.preallocate<ImplExprDiv>(line); break;
		case Kind::Mod: ei.code_buffer.preallocate<ImplExprMod>(line); break;
		case Kind::MakeRange: ei.code_buffer.preallocate<ImplExprMakeRange>(line); break;
		case Kind::IndexRead: ei.code_buffer.preallocate<ImplExprIndexRead>(line); break;
		case Kind::IndexWrite:
			assert(third != nullptr);
			ei.code_buffer.preallocate<ImplExprIndexWrite>(line);
			break;
		}

		if (left) left->calculate_size(ei);
		if (right) right->calculate_size(ei);
		if (third) third->calculate_size(ei);
	}

	ImplExpr* AstExprOper2::emit(EmitInfo& ei) {
		switch (kind_) {
		case Kind::LogOr: return make<ImplExprLogOr>(ei, line, left, right);
		case Kind::LogAnd: return make<ImplExprLogAnd>(ei, line, left, right);
		case Kind::BinOr: return make<ImplExprBinOr>(ei, line, left, right);
		case Kind::BinAnd: return make<ImplExprBinAnd>(ei, line, left, right);
		case Kind::BinXor: return make<ImplExprBinXor>(ei, line, left, right);
		case Kind::BinLShift: return make<ImplExprBinLShift>(ei, line, left, right);
		case Kind::BinRShift: return make<ImplExprBinRShift>(ei, line, left, right);
		case Kind::Add: return make<ImplExprAdd>(ei, line, left, right);
		case Kind::Sub: return make<ImplExprSub>(ei, line, left, right);
		case Kind::Mul: return make<ImplExprMul>(ei, line, left, right);
		case Kind::Div: return make<ImplExprDiv>(ei, line, left, right);
		case Kind::Mod: return make<ImplExprMod>(ei, line, left, right);
		case Kind::MakeRange: return make<ImplExprMakeRange>(ei, line, left, right);
		case Kind::IndexRead: return make<ImplExprIndexRead>(ei, line, left, right);
		case Kind::IndexWrite:
			assert(third != nullptr);
			return make<ImplExprIndexWrite>(ei, line, left, right, third);
		}
		assert(false);
		return nullptr;
	}

	void AstExprOper2::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOper2::visit_children(AstVisitor& vis) {
		if (left) left->visit(vis);
		if (right) right->visit(vis);
		if (third) third->visit(vis);
	}
	void AstExprOper2::update_value_to_write(Kind new_kind, std::unique_ptr<AstExpr> third) {
		kind_ = new_kind;
		this->third = std::move(third);
	}
}