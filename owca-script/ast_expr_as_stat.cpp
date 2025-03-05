#include "stdafx.h"
#include "ast_expr_as_stat.h"
#include "vm.h"

namespace OwcaScript::Internal {
	class ImplExprAsStat : public ImplStat {
	public:
		using ImplStat::ImplStat;

		#define FIELDS(Q) \
			Q(value, ImplExpr*)

		IMPL_DEFINE_STAT(Kind::ExprAsStat)

		void execute_statement_impl(OwcaVM vm) const override {
			value->execute_expression(vm);
		}
		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
			VM::get(vm).update_execution_line(line);
			auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };
				value->execute_expression(vm);
			co_return;
		}
		size_t calculate_generator_allocation_size() const override {
			return calculate_generator_object_size_for_this();
		}
	};

	void AstExprAsStat::calculate_size(CodeBufferSizeCalculator &ei) const {
		ei.code_buffer.preallocate<ImplExprAsStat>(line);
		child->calculate_size(ei);
	}
	ImplStat* AstExprAsStat::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplExprAsStat>(line);
		auto val = child->emit(ei);
		ret->init(val);
		return ret;
	}
	void AstExprAsStat::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstExprAsStat::visit_children(AstVisitor& vis) {
		child->visit(vis);
	}
	void AstExprAsStat::initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplStat::Kind::ExprAsStat] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprAsStat>(line); };
	}
}