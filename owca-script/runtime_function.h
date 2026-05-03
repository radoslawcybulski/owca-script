#ifndef RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H
#define RC_OWCA_SCRIPT_RUNTIME_FUNCTION_H

#include "owca-script/executor.h"
#include "stdafx.h"
#include "allocation_base.h"
#include "ast_function.h"
#include "owca_vm.h"
#include "owca_code.h"
#include "owca_value.h"
#include "exec_buffer.h"
#include "executor.h"

namespace OwcaScript {
	namespace Internal {
		struct RuntimeFunction : public AllocationBase {
			static constexpr const Kind object_kind = Kind::RuntimeFunction;

			OwcaCode code;
			std::string_view name, full_name;
			std::uint16_t param_count = 0;
			std::uint16_t max_states = 0;
			std::uint16_t max_temporaries = 0;
			std::uint16_t max_values = 0; 
			const bool is_method = false;
			const bool is_generator = false;

			std::string_view type() const override;
			std::string to_string() const override;

			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;

			virtual OwcaValue call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) = 0;
			virtual unsigned int line(ExecuteBufferReader::Position) const;
		protected:
			RuntimeFunction(OwcaCode code, std::string_view name, std::string_view full_name, bool is_method, bool is_generator) :
				code(std::move(code)), name(name), full_name(full_name), is_method(is_method), is_generator(is_generator) {}
		};

		struct RuntimeFunctionScript : public RuntimeFunction {
			std::vector<OwcaValue> values_from_parents;
			std::vector<AstFunction::CopyFromParent> copy_from_parents;
			std::vector<std::string_view> identifier_names;
			ExecuteBufferReader::Position entry_point{ 0 };
		protected:
			RuntimeFunctionScript(OwcaCode code, std::string_view name, std::string_view full_name, bool is_method, bool is_generator, ExecuteBufferReader::Position entry_point) : 
					RuntimeFunction(std::move(code), name, full_name, is_method, is_generator), entry_point(entry_point) {}
		};
		struct RuntimeFunctionScriptFunction : public RuntimeFunctionScript {

			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;

			RuntimeFunctionScriptFunction(OwcaCode code, std::string_view name, std::string_view full_name, bool is_method, ExecuteBufferReader::Position entry_point) : RuntimeFunctionScript(code, name, full_name, is_method, false, entry_point) {}

			OwcaValue call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) override;
		};

		struct RuntimeFunctionScriptGenerator : public RuntimeFunctionScript {
			void gc_mark(OwcaVM vm, GenerationGC generation_gc) const override;

			RuntimeFunctionScriptGenerator(OwcaCode code, std::string_view name, std::string_view full_name, bool is_method, ExecuteBufferReader::Position entry_point) : RuntimeFunctionScript(code, name, full_name, is_method, true, entry_point) {}

			OwcaValue call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) override;
		};

		struct RuntimeFunctionNativeFunction : public RuntimeFunction {
			std::vector<std::string_view> parameter_names;
			NativeCodeProvider::Function function;
			unsigned int line_;

			RuntimeFunctionNativeFunction(OwcaCode code, std::string_view name, std::string_view full_name, bool is_method, unsigned int line) : RuntimeFunction(code, name, full_name, is_method, false), line_(line) {}

			OwcaValue call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) override;
			unsigned int line(ExecuteBufferReader::Position) const override { return line_; }
		};

		struct RuntimeFunctionNativeGenerator : public RuntimeFunction {
			std::vector<std::string_view> parameter_names;
			NativeCodeProvider::GeneratorFunction generator;
			unsigned int line_;

			RuntimeFunctionNativeGenerator(OwcaCode code, std::string_view name, std::string_view full_name, bool is_method, unsigned int line) : RuntimeFunction(code, name, full_name, is_method, true), line_(line) {}

			Generator run_native_generator(Executor &e, Iterator *iter_object, Generator generator_object);
			OwcaValue call(Executor &e, Executor::TemporariesPtr temporary_ptr, Executor::StatesTypePtr states_ptr) override;
			unsigned int line(ExecuteBufferReader::Position) const override { return line_; }
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
