#ifndef RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H
#define RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H

#include "stdafx.h"
#include "allocation_base.h"
#include "ast_function.h"
#include "owca_vm.h"
#include "owca_code.h"
#include "owca_value.h"
#include "exec_buffer.h"

namespace OwcaScript {
	namespace Internal {
		struct RuntimeFunction : public AllocationBase {
			static constexpr const Kind object_kind = Kind::RuntimeFunction;

			struct ScriptFunction {
				std::vector<OwcaValue> values_from_parents;
				std::vector<AstFunction::CopyFromParent> copy_from_parents;
				std::vector<std::string_view> identifier_names;
				ExecuteBufferReader::Position entry_point{ 0 };
				bool is_generator = false;
			};
			struct NativeFunction {
				std::vector<std::string_view> parameter_names;
				NativeCodeProvider::Function function;
				unsigned int line;
			};
			struct NativeGenerator {
				std::vector<std::string_view> parameter_names;
				NativeCodeProvider::GeneratorFunction generator;
				unsigned int line;
			};
			OwcaCode code;
			std::variant<ScriptFunction, NativeFunction, NativeGenerator> data;
			std::string_view name, full_name;
			std::uint16_t param_count = 0;
			std::uint16_t max_states = 0;
			std::uint16_t max_temporaries = 0;
			std::uint16_t max_values = 0; 
			bool is_method = false;

			RuntimeFunction(OwcaCode code, std::string_view name, std::string_view full_name, std::variant<ScriptFunction, NativeFunction, NativeGenerator> data);

			template <typename ... F> auto visit(F &&...fns) { return visit_variant(data, std::forward<F>(fns)...); }
			template <typename ... F> auto visit(F &&...fns) const { return visit_variant(data, std::forward<F>(fns)...); }
			bool is_generator() const;
			
			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;
		};

		struct RuntimeFunctions : public AllocationBase {
			static constexpr const Kind object_kind = Kind::RuntimeFunctions;

			std::array<RuntimeFunction*, 16> functions;
			std::string_view name, full_name;

			RuntimeFunctions(std::string_view name, std::string_view full_name) : name(name), full_name(full_name) {
				for(auto &f : functions) f = nullptr;
			}

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
