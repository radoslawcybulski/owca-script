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

				ScriptFunction();
			};
			struct NativeFunction {
				std::span<std::string_view> parameter_names;
				NativeCodeProvider::Function function;
			};
			struct NativeGenerator {
				std::span<std::string_view> parameter_names;
				NativeCodeProvider::GeneratorFunction generator;
			};
			std::shared_ptr<CodeBuffer> code;
			std::variant<ScriptFunction, NativeFunction, NativeGenerator> data;
			std::string_view name, full_name;
			Line fileline;
			unsigned int param_count = 0;
			bool is_method = false;

			RuntimeFunction(std::shared_ptr<CodeBuffer> code, std::string_view name, std::string_view full_name, Line fileline, unsigned int param_count, bool is_method);

			template <typename ... F> auto visit(F &&...fns) { return visit_variant(data, std::forward<F>(fns)...); }
			template <typename ... F> auto visit(F &&...fns) const { return visit_variant(data, std::forward<F>(fns)...); }
			bool is_generator() const;
			
			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;
		};

		struct RuntimeFunctions : public AllocationBase {
			static constexpr const Kind object_kind = Kind::RuntimeFunctions;

			std::unordered_map<unsigned int, RuntimeFunction*> functions;
			std::string_view name, full_name;

			RuntimeFunctions(std::string_view name, std::string_view full_name) : name(name), full_name(full_name) {}

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;
		};

		struct BoundFunctionSelfObject : public AllocationBase {
			static constexpr const Kind object_kind = Kind::BoundSelfObject;

			OwcaValue self;

			BoundFunctionSelfObject(OwcaValue self) : self(std::move(self)) {}

			std::string_view type() const override { return "bound function's self helper object"; }
			std::string to_string() const override { return std::string{ type() }; }
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;
			BoundFunctionSelfObject* is_bound_function_self_object() override { return this; }
		};
	}
}

#endif
