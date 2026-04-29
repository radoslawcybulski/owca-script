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
	// void AstFunction::CopyFromParent::serialize_object(Serializer &ser) const {
	// 	ser.serialize(index_in_parent);
	// 	ser.serialize(index_in_child);
	// }
	// void AstFunction::CopyFromParent::deserialize_object(Deserializer &ser) {
	// 	ser.deserialize(index_in_parent);
	// 	ser.deserialize(index_in_child);
	// }

	// class ImplExprScriptFunction : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#define FIELDS(Q) \
	// 		Q(copy_from_parents, std::span<AstFunction::CopyFromParent>) \
	// 		Q(identifier_names, std::span<std::string_view>) \
	// 		Q(name, std::string_view) \
	// 		Q(full_name, std::string_view) \
	// 		Q(is_generator, bool) \
	// 		Q(param_count, unsigned int) \
	// 		Q(body, ImplStat *)
    
	// 	IMPL_DEFINE_EXPR(Kind::ScriptFunction)


	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override
	// 	{
	// 		RuntimeFunction::ScriptFunction sf;
	// 		sf.body = body;
	// 		auto code = VM::get(vm).currently_running_code();
	// 		assert(code);
	// 		sf.copy_from_parents = copy_from_parents;
	// 		sf.identifier_names = identifier_names;
	// 		sf.is_generator = is_generator;

	// 		sf.values_from_parents.reserve(copy_from_parents.size());
	// 		for (auto c : copy_from_parents) {
	// 			sf.values_from_parents.push_back(VM::get(vm).get_identifier(c.index_in_parent));
	// 		}
	// 		auto rf = VM::get(vm).allocate<RuntimeFunction>(0, std::move(code), name, full_name, line, param_count, !identifier_names.empty() && identifier_names[0] == "self");
	// 		rf->data = std::move(sf);
	// 		auto rfs = VM::get(vm).allocate<RuntimeFunctions>(0, name, full_name);
	// 		rfs->functions[rf->param_count] = rf;
	// 		auto of = OwcaFunctions{ rfs };

	// 		return of;
	// 	}
	// };
	// class ImplExprNativeFunction : public ImplExpr {
	// public:
	// 	using ImplExpr::ImplExpr;

	// 	#undef FIELDS
	// 	#define FIELDS(Q) \
	// 		Q(parameter_names, std::span<std::string_view>) \
	// 		Q(name, std::string_view) \
	// 		Q(full_name, std::string_view) \
	// 		Q(is_generator, bool)
    
	// 	IMPL_DEFINE_EXPR(Kind::NativeFunction)

	// 	OwcaValue execute_expression_impl(OwcaVM vm) const override
	// 	{
	// 		auto code = VM::get(vm).currently_running_code();
	// 		assert(code);
	// 		auto rf = VM::get(vm).allocate<RuntimeFunction>(0, std::move(code), name, full_name, line, (unsigned int)parameter_names.size(), !parameter_names.empty() && parameter_names[0] == "self");

	// 		bool filled = false;

	// 		if (auto native = rf->code->native_code_provider()) {
	// 			if (is_generator) {
	// 				auto fnc = native->native_generator(full_name, VM::get(vm).get_currently_building_class(), FunctionToken{ rf }, parameter_names);
	// 				RuntimeFunction::NativeGenerator nf;
	// 				nf.parameter_names = parameter_names;
	// 				if (fnc) {
	// 					nf.generator = std::move(*fnc);
	// 				}
	// 				else {
	// 					VM::get(vm).throw_missing_native(std::format("missing native function {}", full_name));
	// 				}
	// 				rf->data = std::move(nf);
	// 			}
	// 			else {
	// 				auto fnc = native->native_function(full_name, VM::get(vm).get_currently_building_class(), FunctionToken{ rf }, parameter_names);
	// 				RuntimeFunction::NativeFunction nf;
	// 				nf.parameter_names = parameter_names;
	// 				if (fnc) {
	// 					nf.function = std::move(*fnc);
	// 				}
	// 				else {
	// 					VM::get(vm).throw_missing_native(std::format("missing native function {}", full_name));
	// 				}
	// 				rf->data = std::move(nf);
	// 			}
	// 		}

	// 		auto rfs = VM::get(vm).allocate<RuntimeFunctions>(0, name, full_name);
	// 		rfs->functions[rf->param_count] = rf;
	// 		auto of = OwcaFunctions{ rfs };

	// 		return of;
	// 	}
	// };

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