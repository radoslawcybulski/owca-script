#include "stdafx.h"
#include "ast_try.h"
#include "vm.h"
#include "owca_value.h"

namespace OwcaScript::Internal {
	void AstTry::emit(EmitInfo& ei) {
        assert(ei.stack.empty());
        ei.states.push();
        auto start = ei.code_writer.position();
        ei.code_writer.append(line, ExecuteOp::TryInit);
        auto body_start = ei.code_writer.append_jump_placeholder(line);
        auto body_end = ei.code_writer.append_jump_placeholder(line);
        std::vector<ExecuteBufferWriter::JumpPlaceholder> fallback_jumps;
        for(auto &c : catches_) {
            for(auto &q : std::get<2>(c)) {
                q->emit(ei);
            }
            ei.stack.pop(std::get<2>(c).size());
            auto &line = std::get<3>(c)->line;
            ei.code_writer.append(line, ExecuteOp::TryCatchType);
            ei.code_writer.append(line, (std::uint32_t)std::get<2>(c).size());
            ei.code_writer.append(line, std::get<1>(c));
            auto skip = ei.code_writer.append_jump_placeholder(line);
            std::get<3>(c)->emit(ei);
            assert(ei.stack.empty());
            ei.code_writer.append(line, ExecuteOp::TryBlockCompleted);
            fallback_jumps.push_back(ei.code_writer.append_jump_placeholder(line));
            ei.code_writer.update_jump_placeholder(skip, ei.code_writer.position());
        }
        ei.code_writer.append(line, ExecuteOp::TryCatchTypeCompleted);
        ei.code_writer.update_jump_placeholder(body_start, ei.code_writer.position());
        assert(ei.stack.empty());
        body_->emit(ei);
        assert(ei.stack.empty());
        ei.code_writer.update_jump_placeholder(body_end, ei.code_writer.position());
        for(auto &j : fallback_jumps) {
            ei.code_writer.update_jump_placeholder(j, ei.code_writer.position());
        }
        ei.code_writer.append(line, ExecuteOp::TryCompleted);
        ei.states.pop();
	}

	void AstTry::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstTry::visit_children(AstVisitor& vis) {
        body_->visit(vis);
        for(auto &c : catches_) {
            for(auto &e : std::get<2>(c)) {
                e->visit(vis);
            }
            std::get<3>(c)->visit(vis);
        }
	}
}