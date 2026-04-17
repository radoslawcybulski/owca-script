#include "stdafx.h"
#include "ast_function.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"
#include "owca_functions.h"
#include "exec_buffer.h"

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

	AstFunction::CopyFromParent::CopyFromParent(ExecuteBufferReader &reader) {
		index_in_parent = reader.decode<std::uint32_t>();
		index_in_child = reader.decode<std::uint32_t>();
	}
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
		ei.code_writer.append(line, (std::uint32_t)param_count_);
		ei.code_writer.append_span_helper(line, identifier_names_);
		if (native_ == Native::Yes) {
		}
		else {
			ei.code_writer.append_span_helper(line, copy_from_parents_);
			auto next = ei.code_writer.append_placeholder<std::uint32_t>(line);

			assert(body_);
			body_->emit(ei);
			ei.code_writer.append(ei.code_writer.current_line(), Internal::ExecuteOp::Return);
			ei.code_writer.update_placeholder(next, ei.code_writer.position());
		}
	}

	void AstFunction::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFunction::visit_children(AstVisitor& vis) {
		if (body_)
			body_->visit(vis);
	}
}