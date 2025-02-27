#include "stdafx.h"
#include "owca_vm.h"
#include "vm.h"
#include "owca_code.h"
#include "ast_compiler.h"

namespace OwcaScript {
	OwcaVM::CompilationFailed::CompilationFailed(std::string filename_, std::vector<OwcaErrorMessage> error_messages_) : filename_(std::move(filename_)), error_messages_(std::move(error_messages_)) {
		err_msg = "compilation of file `" + this->filename_ + "` failed:";
		for(auto &m : this->error_messages_) {
			err_msg += "\n";
			err_msg += m.to_string();
		}
	}

	OwcaVM::OwcaVM() : vm_owner(std::make_shared<Internal::VM>())
	{
		vm = vm_owner.get();
	}

	OwcaVM::~OwcaVM() = default;

	OwcaValue OwcaVM::execute(const OwcaCode &oc, OwcaValue values, OwcaValue *output_dict)
	{
		return vm->execute_code_block(oc, values, output_dict);
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

	OwcaValue OwcaVM::create_array(std::vector<OwcaValue> values) const
	{
		return vm->create_array(std::move(values));
	}
	OwcaValue OwcaVM::create_tuple(std::vector<OwcaValue> values) const
	{
		return vm->create_tuple(std::move(values));
	}
	OwcaValue OwcaVM::create_map(const std::vector<OwcaValue> &values) const
	{
		return vm->create_map(values);
	}
	OwcaValue OwcaVM::create_map(const std::vector<std::pair<OwcaValue, OwcaValue>> &values) const
	{
		return vm->create_map(values);
	}
	OwcaValue OwcaVM::create_map(const std::vector<std::pair<std::string, OwcaValue>> &values) const
	{
		return vm->create_map(values);
	}
	OwcaValue OwcaVM::create_set(const std::vector<OwcaValue> &values) const
	{
		return vm->create_set(values);
	}
	OwcaValue OwcaVM::create_string_from_view(std::string_view txt) const
	{
		return vm->create_string_from_view(txt);
	}
	OwcaValue OwcaVM::create_string(std::string txt) const
	{
		return vm->create_string(txt);
	}
	OwcaValue OwcaVM::create_string(OwcaValue str, size_t start, size_t end) const
	{
		return vm->create_string(str, start, end);
	}
	OwcaValue OwcaVM::create_string(OwcaValue str, size_t count) const
	{
		return vm->create_string(str, count);
	}
	OwcaValue OwcaVM::create_string(OwcaValue left, OwcaValue right) const
	{
		return vm->create_string(left, right);
	}
	void OwcaVM::run_gc() {
		vm->run_gc();
	}

	void OwcaVM::gc_mark(OwcaValue v, GenerationGC ggc)
	{
		vm->gc_mark(v, ggc);
	}
}
