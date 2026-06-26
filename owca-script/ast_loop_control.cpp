#include "stdafx.h"
#include "ast_loop_control.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	void AstLoopControl::emit(EmitInfo& ei) {
		ei.code_writer.append(line, ExecuteOp::Jump);
		auto pos = ei.code_writer.append_jump_placeholder(line);
		assert(ei.stack.empty());
		bool found = false;
		for(auto &le : ei.break_loops) {
			if (le.depth == depth_) {
				switch(mode_) {
				case Mode::Break:
					le.break_positions.push_back(pos);
					break;
				case Mode::Continue:
					le.continue_positions.push_back(pos);
					break;
				}
				found = true;
				break;
			}
		}
		assert(found);
	}

	void AstLoopControl::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstLoopControl::visit_children(AstVisitor& vis) {
	}
}