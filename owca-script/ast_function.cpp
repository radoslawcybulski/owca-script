#include "stdafx.h"
#include "ast_function.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"
#include "owca_functions.h"
#include "exec_buffer.h"
#include "ast_compiler.h"

namespace OwcaScript::Internal {
	void serialize_object(ExecuteBufferWriter &writer, Line line, const AstFunction::CopyFromParent &o) {
		writer.append(line, o.index_in_parent);
		writer.append(line, o.index_in_child);
	}

	void AstFunction::emit(EmitInfo& ei) {
		ei.code_writer.append(line, ExecuteOp::Function);
		ei.code_writer.append(line, name_);
		ei.code_writer.append(line, full_name_);
		ei.code_writer.append(line, native_ == Native::Yes);
		ei.code_writer.append(line, generator_ == Generator::Yes);
		ei.code_writer.append(line, param_count_ > 0 && identifier_names_[0] == "self");
		ei.code_writer.append(line, (std::uint16_t)param_count_);
		ei.code_writer.append(line, (std::uint16_t)identifier_names_.size());
		auto temporaries_count = ei.code_writer.append_placeholder<std::uint16_t>(line);
		auto states_count = ei.code_writer.append_placeholder<std::uint16_t>(line);

		for(auto &id : identifier_names_) {
			ei.code_writer.append(line, id);
		}
		if (native_ == Native::Yes) {
			ei.code_writer.update_placeholder(temporaries_count, (std::uint16_t)0);
			ei.code_writer.update_placeholder(states_count, (std::uint16_t)0);
		}
		else {
			ei.code_writer.append(line, (std::uint32_t)copy_from_parents_.size());
			for(auto i = 0u; i < copy_from_parents_.size(); ++i) {
				ei.code_writer.append(line, copy_from_parents_[i].index_in_parent);
				ei.code_writer.append(line, copy_from_parents_[i].index_in_child);
			}
			auto next = ei.code_writer.append_jump_placeholder(line);

			EmitInfo::MaxCounter us_stack, us_states;
			bool us_generator = generator_ == Generator::Yes;
			std::swap(us_stack, ei.stack);
			std::swap(us_states, ei.states);
			std::swap(us_generator, ei.generator);
			assert(body_);
			body_->emit(ei);
			assert(ei.stack.empty());
			assert(ei.states.empty());
			if (ei.stack.maximum() > std::numeric_limits<std::uint16_t>::max()) {
				ei.compiler.add_error(OwcaErrorKind::TooManyValues, ei.compiler.filename(), line, "too many values - maximum is 65535");
			}
			if (ei.states.maximum() > std::numeric_limits<std::uint16_t>::max()) {
				ei.compiler.add_error(OwcaErrorKind::TooManyStates, ei.compiler.filename(), line, "too many states - maximum is 65535");
			}
			ei.code_writer.update_placeholder(temporaries_count, (std::uint16_t)ei.stack.maximum());
			ei.code_writer.update_placeholder(states_count, (std::uint16_t)ei.states.maximum() + 1);
			std::swap(us_stack, ei.stack);
			std::swap(us_states, ei.states);
			std::swap(us_generator, ei.generator);
			ei.code_writer.append(ei.code_writer.current_line(), generator_ == Generator::Yes ? Internal::ExecuteOp::ReturnCloseIterator : Internal::ExecuteOp::Return);
			ei.code_writer.update_jump_placeholder(next, (std::int32_t)ei.code_writer.position());
		}
		ei.stack.push();
	}

	void AstFunction::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFunction::visit_children(AstVisitor& vis) {
		if (body_)
			body_->visit(vis);
	}
}