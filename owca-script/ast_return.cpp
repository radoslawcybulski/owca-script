#include "stdafx.h"
#include "ast_return.h"
#include "vm.h"
#include "owca_value.h"
#include "flow_control.h"

namespace OwcaScript::Internal {
	void AstReturn::emit(EmitInfo& ei) {
		if (value_) {
			auto ret = value_->emit(ei);
			if (ei.per_function.generator) {
				ei.code_writer.append(line, ExecuteOp::Yield);
				ei.code_writer.append(line, ret.index);
				ei.code_writer.append(line, ExecuteOp::ReturnCloseIterator);
			}
			else {
				ei.code_writer.append(line, ExecuteOp::ReturnValue);
				ei.code_writer.append(line, ret.index);
			}
		}
		else {
			ei.code_writer.append(line, ExecuteOp::Return);
		}
	}

	void AstReturn::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstReturn::visit_children(AstVisitor& vis) {
		if (value_)
			value_->visit(vis);
	}
}