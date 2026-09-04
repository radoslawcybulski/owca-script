#include "stdafx.h"
#include "ast_try.h"
#include "vm.h"
#include "owca_value.h"
#include "exec_buffer.h"

namespace OwcaScript::Internal {
	void AstTry::emit(EmitInfo& ei) {
        std::vector<ExecuteBufferWriter::JumpPlaceholder> final_placeholders;

        ei.code_writer.append(line, ExecuteOp::Try);
        auto catch_pos = ei.code_writer.append_jump_placeholder(line);
        auto end_try_pos = ei.code_writer.append_jump_placeholder(line);

        body_->emit(ei);

        ei.code_writer.append(line, ExecuteOp::Jump);
        auto jump_pos = ei.code_writer.append_jump_placeholder(line);
        final_placeholders.push_back(jump_pos);
        ei.code_writer.update_jump_placeholder(catch_pos, (std::int32_t)ei.code_writer.position());

        for(auto & [name, var_index, exprs, stat] : catches_) {
            std::vector<TempInfo> arg_temps;
            for(auto &q : exprs) {
                arg_temps.push_back(q->emit(ei));
            }
            ei.code_writer.append(line, ExecuteOp::TryCatchType);
            auto end_catch = ei.code_writer.append_jump_placeholder(line);
            ei.code_writer.append(line, (std::uint32_t)exprs.size());
            ei.code_writer.append(line, var_index);
            for(auto &t : arg_temps) {
                ei.code_writer.append(line, t.index);
            }

            stat->emit(ei);

            ei.code_writer.append(line, ExecuteOp::TryCatchCompleted);
            auto jump_pos = ei.code_writer.append_jump_placeholder(line);
            final_placeholders.push_back(jump_pos);
            
            ei.code_writer.update_jump_placeholder(end_catch, (std::int32_t)ei.code_writer.position());
        }
        ei.code_writer.append(line, ExecuteOp::TryCatchTypeCompleted);

        for(auto &fp : final_placeholders) {
            ei.code_writer.update_jump_placeholder(fp, (std::int32_t)ei.code_writer.position());
        }
        ei.code_writer.update_jump_placeholder(end_try_pos, (std::int32_t)ei.code_writer.position());

        // ei.per_function.deferred_actions.push_back([=, this, &ei]() {
        //     ei.per_function.try_with_all_blocks[try_with_index].jump = ei.code_writer.position();

        //     for(auto i = 0u; i < catches_.size(); ++i) {
        //         auto & [name, var_index, exprs, stat] = catches_[i];
        //         std::vector<TempInfo> arg_temps;
        //         for(auto &q : exprs) {
        //             arg_temps.push_back(q->emit(ei));
        //         }
        //         auto &line = stat->line;
        //         ei.code_writer.append(line, ExecuteOp::TryCatchType);
        //         ei.code_writer.append_jump_position(line, catch_positions[i]);
        //         ei.code_writer.append_jump_position(line, catch_positions_end[i]);
        //         ei.code_writer.append(line, (std::uint32_t)exprs.size());
        //         ei.code_writer.append(line, var_index);
        //         for(auto &t : arg_temps) {
        //             ei.code_writer.append(line, t.index);
        //         }
        //     }
        //     if (ei.per_function.try_with_all_blocks[try_with_index].next == 0xffffffff) {
        //         ei.code_writer.append(line, ExecuteOp::TryCatchTypeCompleted);
        //     }
        // });
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