#include "stdafx.h"
#include "owca_vm.h"
#include "vm.h"
#include "owca_code.h"
#include "ast_compiler.h"

namespace OwcaScript {
	OwcaVM::OwcaVM() : vm_owner(std::make_shared<Internal::VM>())
	{
		vm = vm_owner.get();
	}

	OwcaVM::~OwcaVM() = default;

	OwcaValue OwcaVM::execute(const OwcaCode&oc, OwcaValue &output_dict)
	{
		return execute(oc, {}, output_dict);
	}
	OwcaValue OwcaVM::execute(const OwcaCode &oc, const std::unordered_map<std::string, OwcaValue>& values)
	{
		return vm->execute_code_block(oc, &values, nullptr);
	}

	OwcaValue OwcaVM::execute(const OwcaCode &oc, const std::unordered_map<std::string, OwcaValue>& values, OwcaValue &output_dict)
	{
		return vm->execute_code_block(oc, &values, &output_dict);
	}

	OwcaValue OwcaVM::execute(const OwcaCode &oc)
	{
		return vm->execute_code_block(oc, nullptr, nullptr);
	}

	OwcaCode OwcaVM::compile(std::string filename, std::string content, std::unique_ptr<NativeCodeProvider> native_code_provider)
	{
		return compile(std::move(filename), std::move(content), {}, std::move(native_code_provider));
	}

	OwcaCode OwcaVM::compile(std::string filename, std::string content, std::span<const std::string> additional_variables, std::unique_ptr<NativeCodeProvider> native_code_provider)
	{
		auto compiler = Internal::AstCompiler{ *vm, std::move(filename), std::move(content), std::move(native_code_provider) };
		auto v = compiler.compile(additional_variables);
		if (!v)
			throw CompilationFailed{ compiler.filename(), compiler.take_error_messages()};

		return OwcaCode{ std::move(v) };
	}

	void OwcaVM::run_gc() {
		vm->run_gc();
	}

	void OwcaVM::gc_mark(const OwcaValue&v, GenerationGC ggc)
	{
		vm->gc_mark(v, ggc);
	}
}
