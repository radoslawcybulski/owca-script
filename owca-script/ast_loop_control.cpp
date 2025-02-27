#include "stdafx.h"
#include "ast_loop_control.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	class ImplLoopControlBreak : public ImplStat {
	public:
		using ImplStat::ImplStat;

		unsigned int depth;

		void init(unsigned int depth) {
			this->depth = depth;
		}

		void execute_statement_impl(OwcaVM vm) const override{
			throw FlowControlBreak{ depth };
		}
	};
	class ImplLoopControlContinue : public ImplStat {
	public:
		using ImplStat::ImplStat;

		unsigned int depth;

		void init(unsigned int depth) {
			this->depth = depth;
		}

		void execute_statement_impl(OwcaVM vm) const override{
			throw FlowControlContinue{ depth };
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
}