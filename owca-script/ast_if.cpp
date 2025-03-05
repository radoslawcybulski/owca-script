#include "stdafx.h"
#include "ast_if.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	class ImplIf : public ImplStat {
	public:
		using ImplStat::ImplStat;

		#define FIELDS(Q) \
			Q(value, ImplExpr*) \
			Q(if_true, ImplStat*) \
			Q(if_false, ImplStat*)
    
		IMPL_DEFINE_STAT(Kind::If)

		void execute_statement_impl(OwcaVM vm) const override{
            auto v = value->execute_expression(vm);
            auto condition = VM::get(vm).calculate_if_true(v);
            if (condition) {
                if_true->execute_statement(vm);
            }
            else if (if_false) {
                if_false->execute_statement(vm);
            }
		}
		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
			VM::get(vm).update_execution_line(line);
			auto pp = VM::AllocatedObjectsPointer{ VM::get(vm) };

			auto v = value->execute_expression(vm);
            auto condition = VM::get(vm).calculate_if_true(v);
            if (condition) {
                co_await if_true->execute_generator_statement(vm, st);
            }
            else if (if_false) {
                co_await if_false->execute_generator_statement(vm, st);
            }
            co_return;
		}
		size_t calculate_generator_allocation_size() const override {
			auto sz = std::max(if_true->calculate_generator_allocation_size(), if_false ? if_false->calculate_generator_allocation_size() : (size_t)0);
			return sz + calculate_generator_object_size_for_this();
		}
	};

	void AstIf::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		ei.code_buffer.preallocate<ImplIf>(line);
        value->calculate_size(ei);
        if_true->calculate_size(ei);
        if (if_false) if_false->calculate_size(ei);
	}
	ImplStat* AstIf::emit(EmitInfo& ei) {
		auto ret = ei.code_buffer.preallocate<ImplIf>(line);
		ImplExpr* v = value->emit(ei);
		ImplStat *if_t = this->if_true->emit(ei);
        ImplStat *if_f = nullptr;
        if (if_false) if_f = if_false->emit(ei);
		ret->init(v, if_t, if_f);
		return ret;
	}

	void AstIf::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstIf::visit_children(AstVisitor& vis) {
        value->visit(vis);
        if_true->visit(vis);
        if (if_false) if_false->visit(vis);
	}
	void AstIf::initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplStat::Kind::If] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplIf>(line); };
	}
}