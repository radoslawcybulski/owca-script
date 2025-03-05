#include "stdafx.h"
#include "ast_function.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"
#include "owca_functions.h"

namespace OwcaScript::Internal {
	void AstFunction::CopyFromParent::serialize_object(Serializer &ser) const {
		ser.serialize(index_in_parent);
		ser.serialize(index_in_child);
	}
	void AstFunction::CopyFromParent::deserialize_object(Deserializer &ser) {
		ser.deserialize(index_in_parent);
		ser.deserialize(index_in_child);
	}

	class ImplExprScriptFunction : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#define FIELDS(Q) \
			Q(copy_from_parents, std::span<AstFunction::CopyFromParent>) \
			Q(identifier_names, std::span<std::string_view>) \
			Q(name, std::string_view) \
			Q(full_name, std::string_view) \
			Q(is_generator, bool) \
			Q(param_count, unsigned int) \
			Q(body, ImplStat *)
    
		IMPL_DEFINE_EXPR(Kind::ScriptFunction)


		OwcaValue execute_expression_impl(OwcaVM vm) const override
		{
			RuntimeFunction::ScriptFunction sf;
			sf.body = body;
			auto code = VM::get(vm).currently_running_code();
			assert(code);
			sf.copy_from_parents = copy_from_parents;
			sf.identifier_names = identifier_names;
			sf.is_generator = is_generator;

			sf.values_from_parents.reserve(copy_from_parents.size());
			for (auto c : copy_from_parents) {
				sf.values_from_parents.push_back(VM::get(vm).get_identifier(c.index_in_parent));
			}
			auto rf = VM::get(vm).allocate<RuntimeFunction>(0, std::move(code), name, full_name, line, param_count, !identifier_names.empty() && identifier_names[0] == "self");
			rf->data = std::move(sf);
			auto rfs = VM::get(vm).allocate<RuntimeFunctions>(0, name, full_name);
			rfs->functions[rf->param_count] = rf;
			auto of = OwcaFunctions{ rfs };

			return of;
		}
	};
	class ImplExprNativeFunction : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		#undef FIELDS
		#define FIELDS(Q) \
			Q(parameter_names, std::span<std::string_view>) \
			Q(name, std::string_view) \
			Q(full_name, std::string_view) \
			Q(is_generator, bool)
    
		IMPL_DEFINE_EXPR(Kind::NativeFunction)

		OwcaValue execute_expression_impl(OwcaVM vm) const override
		{
			auto code = VM::get(vm).currently_running_code();
			assert(code);
			auto rf = VM::get(vm).allocate<RuntimeFunction>(0, std::move(code), name, full_name, line, (unsigned int)parameter_names.size(), !parameter_names.empty() && parameter_names[0] == "self");

			bool filled = false;

			if (auto native = rf->code->native_code_provider()) {
				if (is_generator) {
					auto fnc = native->native_generator(full_name, FunctionToken{ rf }, parameter_names);
					RuntimeFunction::NativeGenerator nf;
					nf.parameter_names = parameter_names;
					if (fnc) {
						nf.generator = std::move(*fnc);
					}
					else {
						VM::get(vm).throw_missing_native(std::format("missing native function {}", full_name));
					}
					rf->data = std::move(nf);
				}
				else {
					auto fnc = native->native_function(full_name, FunctionToken{ rf }, parameter_names);
					RuntimeFunction::NativeFunction nf;
					nf.parameter_names = parameter_names;
					if (fnc) {
						nf.function = std::move(*fnc);
					}
					else {
						VM::get(vm).throw_missing_native(std::format("missing native function {}", full_name));
					}
					rf->data = std::move(nf);
				}
			}

			auto rfs = VM::get(vm).allocate<RuntimeFunctions>(0, name, full_name);
			rfs->functions[rf->param_count] = rf;
			auto of = OwcaFunctions{ rfs };

			return of;
		}
	};

	void AstFunction::calculate_size(CodeBufferSizeCalculator &ei) const
	{
		if (native == Native::Yes) {
			ei.code_buffer.preallocate<ImplExprNativeFunction>(line);
			// std::span<std::string_view> parameter_names, OwcaVM::NativeCodeProvider::Function function, std::string_view name
			ei.code_buffer.preallocate_array<std::string_view>(params.size());
			for (auto i = 0u; i < params.size(); ++i) {
				ei.code_buffer.allocate(params[i]);
			}
			ei.code_buffer.allocate(name_);
			ei.code_buffer.allocate(full_name_);
		}
		else {
			assert(body);
			ei.code_buffer.preallocate<ImplExprScriptFunction>(line);
			ei.code_buffer.preallocate_array<CopyFromParent>(copy_from_parents.size());
			ei.code_buffer.preallocate_array<std::string_view>(identifier_names.size());
			for (auto i = 0u; i < identifier_names.size(); ++i) {
				ei.code_buffer.allocate(identifier_names[i]);
			}
			ei.code_buffer.allocate(name_);
			ei.code_buffer.allocate(full_name_);
			body->calculate_size(ei);
		}
	}

	ImplExpr* AstFunction::emit(EmitInfo& ei) {
		if (native == Native::Yes) {
			auto ret = ei.code_buffer.preallocate<ImplExprNativeFunction>(line);
			// std::span<std::string_view> parameter_names, OwcaVM::NativeCodeProvider::Function function, std::string_view name
			auto pn = ei.code_buffer.preallocate_array<std::string_view>(params.size());
			for (auto i = 0u; i < params.size(); ++i) {
				pn[i] = ei.code_buffer.allocate(params[i]);
			}
			auto nm = ei.code_buffer.allocate(name_);
			auto fm = ei.code_buffer.allocate(full_name_);
			ret->init(pn, nm, fm, generator == Generator::Yes);
			return ret;
		}
		else {
			assert(body);
			auto ret = ei.code_buffer.preallocate<ImplExprScriptFunction>(line);
			auto c = ei.code_buffer.preallocate_array<CopyFromParent>(copy_from_parents.size());
			for (auto i = 0u; i < copy_from_parents.size(); ++i) {
				c[i] = copy_from_parents[i];
			}
			auto inm = ei.code_buffer.preallocate_array<std::string_view>(identifier_names.size());
			for (auto i = 0u; i < identifier_names.size(); ++i) {
				inm[i] = ei.code_buffer.allocate(identifier_names[i]);
			}
			auto nm = ei.code_buffer.allocate(name_);
			auto fm = ei.code_buffer.allocate(full_name_);
			auto bd = body->emit(ei);
			ret->init(c, inm, nm, fm, generator == Generator::Yes, (unsigned int)params.size(), bd);
			return ret;
		}
	}

	void AstFunction::visit(AstVisitor& vis) { vis.apply(*this); }
	void AstFunction::visit_children(AstVisitor& vis) {
		if (body)
			body->visit(vis);
	}
	void AstFunction::initialize_serialization_functions(std::span<std::function<ImplExpr*(Deserializer&, Line)>> functions)
	{
		functions[(size_t)ImplExpr::Kind::NativeFunction] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprNativeFunction>(line); };
		functions[(size_t)ImplExpr::Kind::ScriptFunction] = [](Deserializer &ser, Line line) { return ser.allocate_object<ImplExprScriptFunction>(line); };
	}
}