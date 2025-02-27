#include "stdafx.h"
#include "ast_expr_member.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprMemberRead : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		ImplExpr* value;
		std::string_view identifier;

		void init(ImplExpr* value, std::string_view identifier) {
			this->value = value;
			this->identifier = identifier;
		}
		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			auto v = value->execute_expression(vm);
			return VM::get(vm).member(v, std::string{ identifier });
		}
	};
	class ImplExprMemberWrite : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		std::string_view identifier;
		ImplExpr* value, *value_to_write;

		void init(ImplExpr* value, std::string_view identifier, ImplExpr* value_to_write) {
			this->value = value;
			this->value_to_write = value_to_write;
			this->identifier = identifier;
		}
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
			ei.code_buffer.allocate(member);
			value_to_write->calculate_size(ei);
			value->calculate_size(ei);
		}
		else {
			ei.code_buffer.preallocate<ImplExprMemberRead>(line);
			ei.code_buffer.allocate(member);
			value->calculate_size(ei);
		}
	}
	ImplExpr* AstExprMember::emit(EmitInfo& ei) {
		if (value_to_write) {
			auto ret = ei.code_buffer.preallocate<ImplExprMemberWrite>(line);
			auto mem = ei.code_buffer.allocate(member);
			auto vw = value_to_write->emit(ei);
			auto v = value->emit(ei);
			ret->init(v, mem, vw);
			return ret;
		}
		auto ret = ei.code_buffer.preallocate<ImplExprMemberRead>(line);
		auto mem = ei.code_buffer.allocate(member);
		auto v = value->emit(ei);
		ret->init(v, mem);
		return ret;
	}
	void AstExprMember::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprMember::visit_children(AstVisitor& vis) {
		value->visit(vis);
		if (value_to_write)
			value_to_write->visit(vis);
	}
}