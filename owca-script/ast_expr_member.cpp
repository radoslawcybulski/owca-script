#include "stdafx.h"
#include "ast_expr_member.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprMemberRead : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#define FIELDS(Q) \
			Q(value, ImplExpr*) \
			Q(identifier, std::string_view)

		IMPL_DEFINE_EXPR(Kind::MemberRead)

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto v = value->execute_expression(vm);
			return VM::get(vm).member(v, std::string{ identifier });
		}
	};
	class ImplExprMemberWrite : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#undef FIELDS
		#define FIELDS(Q) \
			Q(value, ImplExpr*) \
			Q(identifier, std::string_view) \
			Q(value_to_write, ImplExpr*)

		IMPL_DEFINE_EXPR(Kind::MemberWrite)

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto vw = value_to_write->execute_expression(vm);
			auto v = value->execute_expression(vm);
			VM::get(vm).member(v, std::string{ identifier }, std::move(vw));
			return {};
		}
	};

	void AstExprMember::calculate_size(CodeBufferSizeCalculator& ei) const
	{
		if (value_to_write) {
			ei.code_buffer.preallocate<ImplExprMemberWrite>(line);
			value->calculate_size(ei);
			ei.code_buffer.allocate(member);
			value_to_write->calculate_size(ei);
		}
		else {
			ei.code_buffer.preallocate<ImplExprMemberRead>(line);
			value->calculate_size(ei);
			ei.code_buffer.allocate(member);
		}
	}
	ImplExpr* AstExprMember::emit(EmitInfo& ei) {
		if (value_to_write) {
			auto ret = ei.code_buffer.preallocate<ImplExprMemberWrite>(line);
			auto v = value->emit(ei);
			auto mem = ei.code_buffer.allocate(member);
			auto vw = value_to_write->emit(ei);
			ret->init(v, mem, vw);
			return ret;
		}
		auto ret = ei.code_buffer.preallocate<ImplExprMemberRead>(line);
		auto v = value->emit(ei);
		auto mem = ei.code_buffer.allocate(member);
		ret->init(v, mem);
		return ret;
	}
	void AstExprMember::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprMember::visit_children(AstVisitor& vis) {
		value->visit(vis);
		if (value_to_write)
			value_to_write->visit(vis);
	}
	void AstExprMember::initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplExpr::Kind::MemberRead] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprMemberRead>(line); };
		functions[(size_t)ImplExpr::Kind::MemberWrite] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprMemberWrite>(line); };
	}
}