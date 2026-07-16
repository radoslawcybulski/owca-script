#include "stdafx.h"
#include "ast_function.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"
#include "owca_functions.h"
#include "exec_buffer.h"
#include "ast_compiler.h"
#include <stdexcept>

namespace OwcaScript::Internal {
	void serialize_object(ExecuteBufferWriter &writer, Line line, const AstFunction::CopyFromParent &o) {
		writer.append(line, o.index_in_parent);
		writer.append(line, o.index_in_child);
	}

	AstBase::TempInfo AstFunction::emit(EmitInfo& ei, std::optional<TempInfo> target) {
		assert(!target);
		target = ei.allocate_temporary();

		const bool is_method = param_count_ > 0 && identifier_names_[0] == "self";
		
		ei.code_writer.append(line, ExecuteOp::Function);
		ei.code_writer.append(line, target->index);
		ei.code_writer.append(line, name_);
		ei.code_writer.append(line, full_name_);
		ei.code_writer.append(line, native_ == Native::Yes);
		ei.code_writer.append(line, generator_ == Generator::Yes);
		ei.code_writer.append(line, is_method);
		ei.code_writer.append(line, (std::uint16_t)(param_count_ - (is_method ? 1 : 0)));
		auto value_count = ei.code_writer.append_placeholder<std::uint16_t>(line);
		ei.code_writer.append(line, (std::uint16_t)(identifier_names_[0] == "" ? identifier_names_.size() - 1 : identifier_names_.size()));

		for(auto &id : identifier_names_) {
			if (id == "") continue;
			ei.code_writer.append(line, id);
		}
		if (native_ == Native::Yes) {
			ei.code_writer.update_placeholder(value_count, (std::uint16_t)identifier_names_.size());
		}
		if (native_ != Native::Yes) {
			ei.code_writer.append(line, (std::uint32_t)copy_from_parents_.size());
			for(auto i = 0u; i < copy_from_parents_.size(); ++i) {
				ei.code_writer.append(line, copy_from_parents_[i].index_in_parent);
				ei.code_writer.append(line, copy_from_parents_[i].index_in_child);
			}
			auto next = ei.code_writer.append_jump_placeholder(line);

			EmitInfo::PerFunctionInfo per_function_our;
			per_function_our.generator = generator_ == Generator::Yes;
			per_function_our.local_variables = per_function_our.max_temporaries = (std::uint32_t)identifier_names_.size();
			std::swap(per_function_our, ei.per_function);
			assert(body_);
			body_->emit(ei);
			std::swap(per_function_our, ei.per_function);
			if (per_function_our.max_temporaries > 0xFFFF) {
				throw CompilationError{ line.line, "too many temporaries in function " + std::string{ full_name_ } };
			}
			ei.code_writer.update_placeholder(value_count, (std::uint16_t)per_function_our.max_temporaries);

			ei.code_writer.append(ei.code_writer.current_line(), generator_ == Generator::Yes ? Internal::ExecuteOp::ReturnCloseIterator : Internal::ExecuteOp::Return);
			ei.code_writer.update_jump_placeholder(next, (std::int32_t)ei.code_writer.position());
		}
		return std::move(*target);
	}

	void AstFunction::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFunction::visit_children(AstVisitor& vis) {
		if (body_)
			body_->visit(vis);
	}
}