#include "stdafx.h"
#include "ast_expr_oper_1.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprOper1 : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		ImplExpr* left;

		void init(ImplExpr* left) {
			this->left = left;
		}
	};
	class ImplExprBinNeg : public ImplExprOper1 {
	public:
		using ImplExprOper1::ImplExprOper1;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm);
			auto v = l.convert_to_int(vm);
			return OwcaInt{ ~v };
		}
	};
	class ImplExprLogNot : public ImplExprOper1 {
	public:
		using ImplExprOper1::ImplExprOper1;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm);
			return OwcaBool{ !l.is_true() };
		}
	};
	class ImplExprNegate : public ImplExprOper1 {
	public:
		using ImplExprOper1::ImplExprOper1;

		OwcaValue execute(OwcaVM &vm) const override {
			auto l = left->execute(vm);
			auto [li, lf] = l.get_int_or_float();
			if (li) {
				auto v = -li->internal_value();
				if (v == li->internal_value()) {
					return OwcaFloat{ -(OwcaFloatInternal)li->internal_value() };
				}
				return OwcaInt{ v };
			}
			if (lf) {
				return OwcaFloat{ -lf->internal_value() };
			}
			vm.vm->throw_not_a_number(l.type());
			assert(false);
			return {};
		}
	};

	template <typename T> static ImplExpr* make(AstBase::EmitInfo& ei, Line line, const std::unique_ptr<AstExpr>& left)
	{
		auto ret = ei.code_buffer.preallocate<T>(line);
		auto l = left->emit(ei);
		ret->init(l);
		return ret;

	}

	ImplExpr* AstExprOper1::emit(EmitInfo& ei) {

		switch (kind) {
		case Kind::BinNeg: return make<ImplExprBinNeg>(ei, line, left);
		case Kind::LogNot: return make<ImplExprLogNot>(ei, line, left);
		case Kind::Negate: return make<ImplExprNegate>(ei, line, left);
		}
		assert(false);
		return nullptr;
	}

	void AstExprOper1::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprOper1::visit_children(AstVisitor& vis) {
		left->visit(vis);
	}
}