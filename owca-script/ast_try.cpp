#include "stdafx.h"
#include "ast_try.h"
#include "vm.h"
#include "owca_value.h"
#include "exec_buffer.h"
namespace OwcaScript::Internal {
	void AstTry::emit(EmitInfo& ei) {
        auto try_with_index = ei.per_function.add_try_with_block();
        ei.per_function.try_with_all_blocks[try_with_index].begin = ei.code_writer.position();
        body_->emit(ei);
        ei.per_function.try_with_all_blocks[try_with_index].end = ei.code_writer.position();
        ei.per_function.pop_try_with_block(try_with_index);


        std::vector<std::uint32_t> catch_positions;
        std::vector<ExecuteBufferWriter::JumpPlaceholder> final_placeholders;

        ei.code_writer.append(line, ExecuteOp::Jump);
        auto jump_pos = ei.code_writer.append_jump_placeholder(line);
        final_placeholders.push_back(jump_pos);

        for(auto & [name, var_index, exprs, stat] : catches_) {
            catch_positions.push_back(ei.code_writer.position());
            stat->emit(ei);
            ei.code_writer.append(line, ExecuteOp::TryCatchCompleted);
            auto jump_pos = ei.code_writer.append_jump_placeholder(line);
            final_placeholders.push_back(jump_pos);
        }

        for(auto &fp : final_placeholders) {
            ei.code_writer.update_jump_placeholder(fp, (std::int32_t)ei.code_writer.position());
        }

        ei.per_function.deferred_actions.push_back([=, this, &ei]() {
            ei.per_function.try_with_all_blocks[try_with_index].jump = ei.code_writer.position();

            for(auto i = 0u; i < catches_.size(); ++i) {
                auto & [name, var_index, exprs, stat] = catches_[i];
                std::vector<TempInfo> arg_temps;
                for(auto &q : exprs) {
                    arg_temps.push_back(q->emit(ei));
                }
                auto &line = stat->line;
                ei.code_writer.append(line, ExecuteOp::TryCatchType);
                ei.code_writer.append_jump_position(line, catch_positions[i]);
                ei.code_writer.append(line, (std::uint32_t)exprs.size());
                ei.code_writer.append(line, var_index);
                for(auto &t : arg_temps) {
                    ei.code_writer.append(line, t.index);
                }
            }
            if (ei.per_function.try_with_all_blocks[try_with_index].next == 0xffffffff) {
                ei.code_writer.append(line, ExecuteOp::TryCatchTypeCompleted);
            }
        });
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