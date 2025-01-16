#ifndef RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H
#define RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H

#include "stdafx.h"
#include "allocation_base.h"
#include "ast_function.h"
#include "owca_vm.h"

namespace OwcaScript {
	namespace Internal {
		class CodeBuffer;

		struct RuntimeFunction : public AllocationBase {
			struct ScriptFunction {
				std::vector<OwcaValue> values_from_parents;
				std::span<AstFunction::CopyFromParent> copy_from_parents;
				std::span<std::string_view> identifier_names;
				ImplStat *body = nullptr;
			};
			struct NativeFunction {
				std::span<std::string_view> parameter_names;
				const OwcaVM::NativeCodeProvider::Function *function = nullptr;
			};
			std::shared_ptr<CodeBuffer> code;
			std::variant<ScriptFunction, NativeFunction> data;
			std::string_view name;
			Line fileline;
			unsigned int param_count = 0;
			bool is_method = false;

			RuntimeFunction(std::variant<ScriptFunction, NativeFunction> data, std::shared_ptr<CodeBuffer> code, std::string_view name, Line fileline, unsigned int param_count, bool is_method) :
				code(std::move(code)), data(std::move(data)), name(name), fileline(fileline), param_count(param_count), is_method(is_method) {}

			template <typename ... F> auto visit(F &&...fns) {
				struct overloaded : F... {
					using F::operator()...;
				};
				return std::visit(overloaded{std::forward<F>(fns)...}, data);
			}
			template <typename ... F> auto visit(F &&...fns) const {
				struct overloaded : F... {
					using F::operator()...;
				};
				return std::visit(overloaded{std::forward<F>(fns)...}, data);
			}

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(VM &vm, GenerationGC generation_gc) override;
		};

		struct RuntimeFunctions : public AllocationBase {
			std::unordered_map<unsigned int, RuntimeFunction*> functions;
			std::string_view name;

			RuntimeFunctions(std::string_view name) : name(name) {}

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(VM &vm, GenerationGC generation_gc) override;
		};
	}
}

#endif
