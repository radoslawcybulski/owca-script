#ifndef RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H
#define RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H

#include "owca-script/executor.h"
#include "owca-script/identifier_index.h"
#include "stdafx.h"
#include "allocation_base.h"
#include "ast_function.h"
#include "owca_vm.h"
#include "owca_code.h"
#include "owca_value.h"
#include "exec_buffer.h"
#include "executor.h"
#include "owca_namespace.h"
#include "namespace.h"

namespace OwcaScript {
	namespace Internal {
		struct RuntimeFunction : public AllocationBase {
			static constexpr const Kind object_kind = Kind::RuntimeFunction;

			OwcaCode code;
			OwcaNamespace owning_namespace;
			std::string_view name, full_name;
			IdentifierIndex name_index;
			std::uint16_t param_count = 0;
			std::uint16_t max_values = 0; // all values + temporaries
			const bool is_method = false;
			const bool is_generator = false;

			std::string_view type() const override;
			std::string to_string() const override;

			void gc_mark(GenerationGC generation_gc) const override;

			virtual OwcaValue call(Executor &e) = 0;
			virtual unsigned int line(CodePosition) const;
            OwcaValue bound_function_self_object() override { assert(false); return {}; }
		protected:
			RuntimeFunction(OwcaCode code, OwcaNamespace owning_namespace, IdentifierIndex name_index, std::string_view name, std::string_view full_name, bool is_method, bool is_generator) :
				code(std::move(code)), owning_namespace(owning_namespace), name(name), name_index(name_index), full_name(full_name), is_method(is_method), is_generator(is_generator) {}
		};

		struct TryWithBlockInfo {
			CodePosition begin;
			CodePosition end;
			CodePosition jump;
			std::uint32_t next;
		};
		struct RuntimeFunctionScript : public RuntimeFunction {
		    void gc_mark(GenerationGC generation_gc) const override;

			std::vector<OwcaValue> values_from_parents;
			std::vector<std::string_view> identifier_names;
			std::vector<TryWithBlockInfo> try_with_blocks;
			CodePosition entry_point = CodePosition{};
			GlobalsPtr globals_ptr;
			OwcaValue *constants_ptr;
		protected:
			RuntimeFunctionScript(OwcaCode code, OwcaNamespace owning_namespace, IdentifierIndex name_index, std::string_view name, std::string_view full_name, bool is_method, bool is_generator, CodePosition entry_point, GlobalsPtr globals_ptr) :
					RuntimeFunction(std::move(code), owning_namespace, name_index, name, full_name, is_method, is_generator), entry_point(entry_point), globals_ptr(globals_ptr), constants_ptr(owning_namespace.internal_value()->constants) {}
		};
		struct RuntimeFunctionScriptFunction : public RuntimeFunctionScript {
			void gc_mark(GenerationGC generation_gc) const override;

			RuntimeFunctionScriptFunction(OwcaCode code, OwcaNamespace owning_namespace, GlobalsPtr globals_ptr, IdentifierIndex name_index, std::string_view name, std::string_view full_name, bool is_method, CodePosition entry_point) : RuntimeFunctionScript(std::move(code), owning_namespace, name_index, name, full_name, is_method, false, entry_point, globals_ptr) {}

			OwcaValue call(Executor &e) override;
		};

		struct RuntimeFunctionScriptGenerator : public RuntimeFunctionScript {
			void gc_mark(GenerationGC generation_gc) const override;

			RuntimeFunctionScriptGenerator(OwcaCode code, OwcaNamespace owning_namespace, GlobalsPtr globals_ptr, IdentifierIndex name_index, std::string_view name, std::string_view full_name, bool is_method, CodePosition entry_point) : RuntimeFunctionScript(std::move(code), owning_namespace, name_index, name, full_name, is_method, true, entry_point, globals_ptr) {}

			OwcaValue call(Executor &e) override;
		};

		struct RuntimeFunctionNativeFunction : public RuntimeFunction {
			std::vector<std::string_view> parameter_names;
			NativeCodeProvider::Function function;
			unsigned int line_;

			RuntimeFunctionNativeFunction(OwcaCode code, OwcaNamespace owning_namespace, IdentifierIndex name_index, std::string_view name, std::string_view full_name, bool is_method, unsigned int line) : RuntimeFunction(std::move(code), owning_namespace, name_index, name, full_name, is_method, false), line_(line) {}

			OwcaValue call(Executor &e) override;
			unsigned int line(CodePosition) const override { return line_; }
		};

		struct RuntimeFunctionNativeGenerator : public RuntimeFunction {
			std::vector<std::string_view> parameter_names;
			NativeCodeProvider::GeneratorFunction generator;
			unsigned int line_;

			RuntimeFunctionNativeGenerator(OwcaCode code, OwcaNamespace owning_namespace, IdentifierIndex name_index, std::string_view name, std::string_view full_name, bool is_method, unsigned int line) : RuntimeFunction(std::move(code), owning_namespace, name_index, name, full_name, is_method, true), line_(line) {}

			Generator run_native_generator(Executor &e, Iterator *iter_object, Generator generator_object);
			OwcaValue call(Executor &e) override;
			unsigned int line(CodePosition) const override { return line_; }
		};

		struct RuntimeFunctions : public AllocationBase {
			static constexpr const Kind object_kind = Kind::RuntimeFunctions;

			std::array<RuntimeFunction*, 16> functions;
			std::string_view name, full_name;
			IdentifierIndex name_index;

			RuntimeFunctions(IdentifierIndex name_index, std::string_view name, std::string_view full_name) : name_index(name_index), name(name), full_name(full_name) {
				for(auto &f : functions) f = nullptr;
			}

			std::string_view type() const override;
			std::string to_string() const override;
			void gc_mark(GenerationGC generation_gc) const override;

            OwcaValue bound_function_self_object() override;
		};

		struct BoundFunctionSelfObject : public AllocationBase {
			static constexpr const Kind object_kind = Kind::BoundSelfObject;

			OwcaValue self;

			BoundFunctionSelfObject(OwcaValue self) : self(std::move(self)) {}

			std::string_view type() const override { return "bound function's self helper object"; }
			std::string to_string() const override { return std::string{ type() }; }
			void gc_mark(GenerationGC generation_gc) const override;
			OwcaValue bound_function_self_object() override { return self; }
		};
	}
}

#endif
