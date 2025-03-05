#include "stdafx.h"
#include "ast_expr_identifier.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"

namespace OwcaScript::Internal {
	class ImplExprIdentifier : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#define FIELDS(Q) \
			Q(index, unsigned int) \
			Q(identifier, std::string_view) \
			Q(value_to_write, ImplExpr*) \
			Q(function_write, bool)

		IMPL_DEFINE_EXPR(Kind::Ident)

		OwcaValue execute_expression_impl(OwcaVM vm) const override {
			if (value_to_write) {
				auto v = value_to_write->execute_expression(vm);
				VM::get(vm).set_identifier(index, v, function_write);
				return v;
			}
			return VM::get(vm).get_identifier(index);
		}
	};
	void AstExprIdentifier::calculate_size(CodeBufferSizeCalculator& ei) const
	{
		ei.code_buffer.preallocate<ImplExprIdentifier>(line);
		ei.code_buffer.allocate(identifier_);
		if (value_to_write) value_to_write->calculate_size(ei);
	}

	ImplExpr* AstExprIdentifier::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprIdentifier>(line);
		auto str = ei.code_buffer.allocate(identifier_);
		assert(index != std::numeric_limits<unsigned int>::max());
		ImplExpr* vw = nullptr;
		if (value_to_write) vw = value_to_write->emit(ei);
		ret->init(index, str, vw, function_write);
		return ret;
	}
	void AstExprIdentifier::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprIdentifier::visit_children(AstVisitor& vis) {
		if (value_to_write)
			value_to_write->visit(vis);
	}
	void AstExprIdentifier::initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplExpr::Kind::Ident] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprIdentifier>(line); };
	}
}