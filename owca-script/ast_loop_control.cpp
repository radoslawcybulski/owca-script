#include "stdafx.h"
#include "ast_loop_control.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	class ImplLoopControlBreak : public ImplStat {
	public:
		using ImplStat::ImplStat;

		#define FIELDS(Q) \
			Q(depth, unsigned int)
    
		IMPL_DEFINE_STAT(Kind::Break)

		void execute_statement_impl(OwcaVM vm) const override{
			throw FlowControlBreak{ depth };
		}
		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
			VM::get(vm).update_execution_line(line);
			execute_statement_impl(vm);
            co_return;
		}
		size_t calculate_generator_allocation_size() const override {
			return calculate_generator_object_size_for_this();
		}
	};
	class ImplLoopControlContinue : public ImplStat {
	public:
		using ImplStat::ImplStat;

		#define FIELDS(Q) \
			Q(depth, unsigned int)
    
		IMPL_DEFINE_STAT(Kind::Continue)

		void execute_statement_impl(OwcaVM vm) const override{
			throw FlowControlContinue{ depth };
		}
		Task execute_generator_statement_impl(OwcaVM vm, State &st) const override {
			VM::get(vm).update_execution_line(line);
			execute_statement_impl(vm);
            co_return;
		}
		size_t calculate_generator_allocation_size() const override {
			return calculate_generator_object_size_for_this();
		}
	};

	void AstLoopControl::calculate_size(CodeBufferSizeCalculator &ei) const
	{
        switch(mode) {
        case Mode::Break:
		    ei.code_buffer.preallocate<ImplLoopControlBreak>(line);
            break;
        case Mode::Continue:
		    ei.code_buffer.preallocate<ImplLoopControlContinue>(line);
            break;
        }
	}
	ImplStat* AstLoopControl::emit(EmitInfo& ei) {
        switch(mode) {
        case Mode::Break: {
		    auto ret = ei.code_buffer.preallocate<ImplLoopControlBreak>(line);
            ret->init(depth);
            return ret; }
        case Mode::Continue: {
		    auto ret = ei.code_buffer.preallocate<ImplLoopControlContinue>(line);
            ret->init(depth);
            return ret; }
        }
        throw 1;
		return nullptr;
	}

	void AstLoopControl::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstLoopControl::visit_children(AstVisitor& vis) {
	}
	void AstLoopControl::initialize_serialization_functions(std::span<std::function<ImplStat*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplStat::Kind::Break] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplLoopControlBreak>(line); };
		functions[(size_t)ImplStat::Kind::Continue] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplLoopControlContinue>(line); };
	}
}