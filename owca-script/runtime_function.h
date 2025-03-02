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
			static constexpr const Kind object_kind = Kind::RuntimeFunction;

			struct ScriptFunction {
				std::vector<OwcaValue> values_from_parents;
				std::span<AstFunction::CopyFromParent> copy_from_parents;
				std::span<std::string_view> identifier_names;
				ImplStat *body = nullptr;
				bool is_generator = false;
			};
			struct NativeFunction {
				std::span<std::string_view> parameter_names;
				OwcaVM::NativeCodeProvider::Function function;
			};
			// struct NativeGeneratorFunction {
			// 	std::span<std::string_view> parameter_names;
			// 	OwcaVM::NativeCodeProvider::Generator function;
			// };
			std::shared_ptr<CodeBuffer> code;
			std::variant<ScriptFunction, NativeFunction> data;
			std::string_view name, full_name;
			Line fileline;
			unsigned int param_count = 0;
			bool is_method = false;

			RuntimeFunction(std::shared_ptr<CodeBuffer> code, std::string_view name, std::string_view full_name, Line fileline, unsigned int param_count, bool is_method) :
				code(std::move(code)), name(name), full_name(full_name), fileline(fileline), param_count(param_count), is_method(is_method) {}

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
			bool is_generator() const;
			
			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(VM &vm, GenerationGC generation_gc) override;
		};

		struct RuntimeFunctions : public AllocationBase {
			static constexpr const Kind object_kind = Kind::RuntimeFunctions;

			std::unordered_map<unsigned int, RuntimeFunction*> functions;
			std::string_view name, full_name;

			RuntimeFunctions(std::string_view name, std::string_view full_name) : name(name), full_name(full_name) {}

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(VM &vm, GenerationGC generation_gc) override;
		};

		struct BoundFunctionSelfObject : public AllocationBase {
			static constexpr const Kind object_kind = Kind::BoundSelfObject;

			OwcaValue self;

			BoundFunctionSelfObject(OwcaValue self) : self(std::move(self)) {}

			std::string_view type() const override { return "bound function's self helper object"; }
			std::string to_string() const override { return std::string{ type() }; }
			void gc_mark(VM& vm, GenerationGC generation_gc) override;
			BoundFunctionSelfObject* is_bound_function_self_object() override { return this; }
		};
	}
}

#endif
