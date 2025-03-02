#include "stdafx.h"
#include "ast_function.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"
#include "owca_functions.h"

namespace OwcaScript::Internal {
	class ImplExprScriptFunction : public ImplExpr {
	public:
		using ImplExpr::ImplExpr;

		std::span<AstFunction::CopyFromParent> copy_from_parents;
		std::span<std::string_view> identifier_names;
		std::string_view name, full_name;
		ImplStat *body;
		unsigned int param_count;
		bool is_generator;

		void init(std::span<AstFunction::CopyFromParent> copy_from_parents, std::span<std::string_view> identifier_names, std::string_view name, std::string_view full_name, bool is_generator, unsigned int param_count, ImplStat *body)
		{
			this->copy_from_parents = copy_from_parents;
			this->identifier_names = identifier_names;
			this->name = name;
			this->full_name = full_name;
			this->is_generator = is_generator;
			this->body = body;
			this->param_count = param_count;
		}

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

		std::span<std::string_view> parameter_names;
		std::string_view name, full_name;
		bool is_generator;

		void init(std::span<std::string_view> parameter_names, std::string_view name, std::string_view full_name, bool is_generator)
		{
			this->parameter_names = parameter_names;
			this->name = name;
			this->full_name = full_name;
			this->is_generator = is_generator;
		}

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
			ei.code_buffer.allocate(name_);
			ei.code_buffer.allocate(full_name_);
			ei.code_buffer.preallocate_array<std::string_view>(params.size());
			for (auto i = 0u; i < params.size(); ++i) {
				ei.code_buffer.allocate(params[i]);
			}
		}
		else {
			assert(body);
			ei.code_buffer.preallocate<ImplExprScriptFunction>(line);
			ei.code_buffer.allocate(name_);
			ei.code_buffer.allocate(full_name_);
			ei.code_buffer.preallocate_array<CopyFromParent>(copy_from_parents.size());
			ei.code_buffer.preallocate_array<std::string_view>(identifier_names.size());
			for (auto i = 0u; i < identifier_names.size(); ++i) {
				ei.code_buffer.allocate(identifier_names[i]);
			}
			body->calculate_size(ei);
		}
	}

	ImplExpr* AstFunction::emit(EmitInfo& ei) {
		if (native == Native::Yes) {
			auto ret = ei.code_buffer.preallocate<ImplExprNativeFunction>(line);
			// std::span<std::string_view> parameter_names, OwcaVM::NativeCodeProvider::Function function, std::string_view name
			auto nm = ei.code_buffer.allocate(name_);
			auto fm = ei.code_buffer.allocate(full_name_);
			auto pn = ei.code_buffer.preallocate_array<std::string_view>(params.size());
			for (auto i = 0u; i < params.size(); ++i) {
				pn[i] = ei.code_buffer.allocate(params[i]);
			}
			ret->init(pn, nm, fm, generator == Generator::Yes);
			return ret;
		}
		else {
			assert(body);
			auto ret = ei.code_buffer.preallocate<ImplExprScriptFunction>(line);
			auto nm = ei.code_buffer.allocate(name_);
			auto fm = ei.code_buffer.allocate(full_name_);
			auto c = ei.code_buffer.preallocate_array<CopyFromParent>(copy_from_parents.size());
			for (auto i = 0u; i < copy_from_parents.size(); ++i) {
				c[i] = copy_from_parents[i];
			}
			auto inm = ei.code_buffer.preallocate_array<std::string_view>(identifier_names.size());
			for (auto i = 0u; i < identifier_names.size(); ++i) {
				inm[i] = ei.code_buffer.allocate(identifier_names[i]);
			}
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
}