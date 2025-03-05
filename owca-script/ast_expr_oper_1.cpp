#include "stdafx.h"
#include "ast_expr_oper_1.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprOper1 : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#define FIELDS(Q) \
			Q(left, ImplExpr*)

		IMPL_DEFINE_EXPR(Kind::BinNeg)
	};
	class ImplExprBinNeg : public ImplExprOper1 {
	public:
		using ImplExprOper1::ImplExprOper1;

		Kind kind() const override { return Kind::BinNeg; }
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
			auto v = l.convert_to_int(vm);
			return OwcaInt{ ~v };
		}
	};
	class ImplExprLogNot : public ImplExprOper1 {
	public:
		using ImplExprOper1::ImplExprOper1;

		Kind kind() const override { return Kind::LogNot; }
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
			return OwcaBool{ !l.is_true() };
		}
	};
	class ImplExprNegate : public ImplExprOper1 {
	public:
		using ImplExprOper1::ImplExprOper1;

		Kind kind() const override { return Kind::Negate; }
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto l = left->execute_expression(vm);
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
			VM::get(vm).throw_not_a_number(l.type());
		}
	};

	template <typename T> static ImplExpr* make(AstBase::EmitInfo& ei, Line line, const std::unique_ptr<AstExpr>& left)
	{
		auto ret = ei.code_buffer.preallocate<T>(line);
		auto l = left->emit(ei);
		ret->init(l);
		return ret;

	}

	void AstExprOper1::calculate_size(CodeBufferSizeCalculator& ei) const
	{
		switch (kind) {
		case Kind::BinNeg: ei.code_buffer.preallocate<ImplExprBinNeg>(line); break;
		case Kind::LogNot: ei.code_buffer.preallocate<ImplExprLogNot>(line); break;
		case Kind::Negate: ei.code_buffer.preallocate<ImplExprNegate>(line); break;
		}
		left->calculate_size(ei);
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
	void AstExprOper1::initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplExpr::Kind::BinNeg] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprBinNeg>(line); };
		functions[(size_t)ImplExpr::Kind::LogNot] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprLogNot>(line); };
		functions[(size_t)ImplExpr::Kind::Negate] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprNegate>(line); };
	}
}