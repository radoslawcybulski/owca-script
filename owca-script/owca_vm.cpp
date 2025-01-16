#include "stdafx.h"
#include "owca_vm.h"
#include "vm.h"
#include "owca_code.h"
#include "ast_compiler.h"

namespace OwcaScript {
	OwcaVM::OwcaVM() : vm(std::make_shared<Internal::VM>())
	{
	}

	OwcaVM::~OwcaVM() = default;

	OwcaValue OwcaVM::execute(const OwcaCode &oc, const std::unordered_map<std::string, OwcaValue>& values)
	{
		return vm->execute_code_block(oc, values);
	}

	OwcaCode OwcaVM::compile(std::string filename, std::string content, const NativeCodeProvider &native_code_provider)
	{
		auto compiler = Internal::AstCompiler{ std::move(filename), std::move(content), native_code_provider };
		auto v = compiler.compile();
		if (!v)
			throw CompilationFailed{ compiler.filename(), compiler.take_error_messages()};

		return OwcaCode{ std::move(v) };
	}

	void OwcaVM::run_gc() {
		vm->run_gc();
	}
}
