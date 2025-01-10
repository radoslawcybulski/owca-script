#include "stdafx.h"
#include "ast_expr_oper_2.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprOper2 : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		ImplExpr* left;
		ImplExpr* right;

		void init(ImplExpr* left, ImplExpr* right) {
			this->left = left;
			this->right = right;
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
	class ImplExprAdd : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto [ li, lf ] = left->execute(vm).get_int_or_float();
			auto [ ri, rf ] = right->execute(vm).get_int_or_float();
			auto lfloat = li ? (OwcaFloatInternal)li->internal_value() : lf->internal_value();
			auto rfloat = ri ? (OwcaFloatInternal)ri->internal_value() : rf->internal_value();
			auto val = lfloat + rfloat;

			if (li && ri) {
				auto val2 = li->internal_value() + ri->internal_value();
				if (val == val2) return OwcaInt{ val2 };
			}
			return OwcaFloat{ val };
		}
	};
	class ImplExprSub : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto [ li, lf ] = left->execute(vm).get_int_or_float();
			auto [ ri, rf ] = right->execute(vm).get_int_or_float();
			auto lfloat = li ? (OwcaFloatInternal)li->internal_value() : lf->internal_value();
			auto rfloat = ri ? (OwcaFloatInternal)ri->internal_value() : rf->internal_value();
			auto val = lfloat - rfloat;

			if (li && ri) {
				auto val2 = li->internal_value() - ri->internal_value();
				if (val == val2) return OwcaInt{ val2 };
			}
			return OwcaFloat{ val };
		}
	};
	class ImplExprMul : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto [ li, lf ] = left->execute(vm).get_int_or_float();
			auto [ ri, rf ] = right->execute(vm).get_int_or_float();
			auto lfloat = li ? (OwcaFloatInternal)li->internal_value() : lf->internal_value();
			auto rfloat = ri ? (OwcaFloatInternal)ri->internal_value() : rf->internal_value();
			auto val = lfloat * rfloat;

			if (li && ri) {
				auto val2 = li->internal_value() * ri->internal_value();
				if (val == val2) return OwcaInt{ val2 };
			}
			return OwcaFloat{ val };
		}
	};
	class ImplExprDiv : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto [ li, lf ] = left->execute(vm).get_int_or_float();
			auto [ ri, rf ] = right->execute(vm).get_int_or_float();
			auto lfloat = li ? (OwcaFloatInternal)li->internal_value() : lf->internal_value();
			auto rfloat = ri ? (OwcaFloatInternal)ri->internal_value() : rf->internal_value();
			if (rfloat == 0) {
				vm.vm->throw_division_by_zero();
			}
			auto val = lfloat + rfloat;

			if (li && ri) {
				auto val2 = li->internal_value() + ri->internal_value();
				if (val == val2) return OwcaInt{ val2 };
			}
			return OwcaFloat{ val };
		}
	};
	class ImplExprMod : public ImplExprOper2 {
	public:
		using ImplExprOper2::ImplExprOper2;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm).convert_to_int(vm);
			auto r = right->execute(vm).convert_to_int(vm);
			if (r == 0) {
				vm.vm->throw_mod_division_by_zero();
			}
			return OwcaInt{ l % r };
		}
	};


	template <typename T> static ImplExpr* make(AstBase::EmitInfo& ei, Line line, const std::unique_ptr<AstExpr>& left, const std::unique_ptr<AstExpr>& right)
	{
		auto ret = ei.code_buffer.preallocate<T>(line);
		auto l = left->emit(ei);
		auto r = right->emit(ei);
		ret->init(l, r);
		return ret;

	}

	ImplExpr* AstExprOper2::emit(EmitInfo& ei) {

		switch (kind) {
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
		}
		assert(false);
		return nullptr;
	}

	void AstExprOper2::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOper2::visit_children(AstVisitor& vis) {
		left->visit(vis);
		right->visit(vis);
	}
}