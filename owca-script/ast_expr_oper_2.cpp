#include "stdafx.h"
#include "ast_expr_oper_2.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "owca_value.h"
#include "dictionary.h"

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

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm);
			if (l.is_true()) return l;
			return right->execute(vm);
		}
	};
	class ImplExprLogAnd : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm);
			if (!l.is_true()) return l;
			return right->execute(vm);
		}
	};
	class ImplExprBinOr : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm).convert_to_int(vm);
			auto r = right->execute(vm).convert_to_int(vm);
			return OwcaInt{ l | r };
		}
	};
	class ImplExprBinAnd : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm).convert_to_int(vm);
			auto r = right->execute(vm).convert_to_int(vm);
			return OwcaInt{ l & r };
		}
	};
	class ImplExprBinXor : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm).convert_to_int(vm);
			auto r = right->execute(vm).convert_to_int(vm);
			return OwcaInt{ l ^ r };
		}
	};
	class ImplExprBinLShift : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm).convert_to_int(vm);
			auto r = right->execute(vm).convert_to_int(vm);
			return OwcaInt{ l << r };
		}
	};
	class ImplExprBinRShift : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm).convert_to_int(vm);
			auto r = right->execute(vm).convert_to_int(vm);
			return OwcaInt{ l >> r };
		}
	};
	class ImplExprMakeRange : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm).convert_to_int(vm);
			auto r = right->execute(vm).convert_to_int(vm);
			return OwcaRange{ OwcaInt{ l }, OwcaInt{ r } };
		}
	};
	class ImplExprAdd : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm);
			auto r = right->execute(vm);
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
				return OwcaString{ l.as_string(vm).internal_value() + r.as_string(vm).internal_value() };
			}
			assert(false);
			throw 1;
		}
	};
	class ImplExprSub : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto [li, lf] = left->execute(vm).get_int_or_float();
			auto [ri, rf] = right->execute(vm).get_int_or_float();
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
			assert(false);
			throw 1;
		}
	};
	class ImplExprMul : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		static std::string mul_string(OwcaVM &vm, std::string_view s, OwcaIntInternal mul) {
			if (mul < 0)
				Internal::VM::get(vm).throw_invalid_operand_for_mul_string(std::to_string(mul));
			std::string res;
			res.reserve(s.size() * mul);
			for (OwcaIntInternal i = 0; i < mul; ++i) {
				res += s;
			}
			return res;
		}
		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm);
			auto r = right->execute(vm);
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
				return OwcaString{ mul_string(vm, l.as_string(vm).internal_value(), rr) };
			}
			if ((li || lf) && r.kind() == OwcaValueKind::String) {
				auto ll = l.convert_to_int(vm);
				return OwcaString{ mul_string(vm, r.as_string(vm).internal_value(), ll) };
			}
			assert(false);
			throw 1;
		}
	};
	class ImplExprDiv : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto [ li, lf ] = left->execute(vm).get_int_or_float();
			auto [ ri, rf ] = right->execute(vm).get_int_or_float();
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
			assert(false);
			throw 1;
		}
	};
	class ImplExprMod : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm).convert_to_int(vm);
			auto r = right->execute(vm).convert_to_int(vm);
			if (r == 0) {
				VM::get(vm).throw_mod_division_by_zero();
			}
			return OwcaInt{ l % r };
		}
	};
	class ImplExprIndexRead : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		static void update_key(OwcaVM &vm, OwcaValue& key, OwcaIntInternal size) {
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

		OwcaValue execute(OwcaVM &vm) const override {
			auto v = left->execute(vm);
			auto key = right->execute(vm);

			auto orig_key = key;

			return v.visit(
				[&](const OwcaString& o) -> OwcaValue {
					const auto size = (OwcaIntInternal)o.internal_value().size();
					if (size != o.internal_value().size()) {
						Internal::VM::get(vm).throw_index_out_of_range(std::format("string size {} is too large for OwcaIntInternal size to properly handle indexing, maximum value is {}", o.internal_value().size(), std::numeric_limits<OwcaIntInternal>::max()));
					}
					update_key(vm, key, size);
					return key.visit(
						[&](OwcaInt k) {
							auto v = k.internal_value();
							if (v < 0 || v >= size)
								Internal::VM::get(vm).throw_index_out_of_range(std::format("index value {} is out of range for string of size {}", orig_key, size));
							return OwcaString{ o.internal_value().substr(v, 1) };
						},
						[&](OwcaRange k) {
							auto [v1, v2] = k.internal_values();
							if (v2 <= v1)
								return OwcaString{ "" };
							if (v1 < 0) v1 = 0;
							if (v2 > size) v2 = size;
							return OwcaString{ o.internal_value().substr(v1, v2 - v1) };
						},
						[&](const auto&) -> OwcaString {
							Internal::VM::get(vm).throw_value_not_indexable(v.type(), key.type());
							assert(false);
							return {};
						}
					);
				},
				[&](const OwcaMap& o) -> OwcaValue {
					return o.dictionary->dict.read(vm, key);
				},
				[&](const auto&) -> OwcaValue {
					Internal::VM::get(vm).throw_value_not_indexable(v.type());
					assert(false);
					return {};
				}
			);
		}
	};
	class ImplExprIndexWrite : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto v = left->execute(vm);
			auto key = right->execute(vm);
			auto value = third->execute(vm);

			auto orig_key = key;

			return v.visit(
				[&](const OwcaMap& o) -> OwcaValue {
					o.dictionary->dict.write(vm, key, std::move(value));
					return {};
				},
				[&](const auto&) -> OwcaValue {
					Internal::VM::get(vm).throw_value_not_indexable(v.type());
					assert(false);
					return {};
				}
			);
		}
	};


	template <typename T> static ImplExpr* make(AstBase::EmitInfo& ei, Line line, const std::unique_ptr<AstExpr>& left, const std::unique_ptr<AstExpr>& right, const std::unique_ptr<AstExpr>& third = nullptr)
	{
		auto ret = ei.code_buffer.preallocate<T>(line);
		auto l = left->emit(ei);
		auto r = right->emit(ei);
		ImplExpr* t = nullptr;
		if (third) t = third->emit(ei);
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

		left->calculate_size(ei);
		right->calculate_size(ei);
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
		left->visit(vis);
		right->visit(vis);
		if (third)
			third->visit(vis);
	}
	void AstExprOper2::update_value_to_write(Kind new_kind, std::unique_ptr<AstExpr> third) {
		kind_ = new_kind;
		this->third = std::move(third);
	}
}