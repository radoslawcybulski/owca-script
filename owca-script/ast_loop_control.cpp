#include "stdafx.h"
#include "ast_loop_control.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	void AstLoopControl::emit(EmitInfo& ei) {
		assert(ei.stack.empty());
        switch(mode_) {
        case Mode::Break:
			ei.code_writer.append(line, ExecuteOp::LoopControlBreak);
			ei.code_writer.append(line, depth_);
			break;
        case Mode::Continue:
			ei.code_writer.append(line, ExecuteOp::LoopControlContinue);
			ei.code_writer.append(line, depth_);
			break;
        }
	}

	void AstLoopControl::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstLoopControl::visit_children(AstVisitor& vis) {
	}
}