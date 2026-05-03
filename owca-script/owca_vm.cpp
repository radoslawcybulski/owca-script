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

	OwcaValue OwcaVM::execute(const OwcaCode &oc) {
		return vm->execute_code_block(oc, std::nullopt, nullptr);
	}
	OwcaValue OwcaVM::execute(const OwcaCode &oc, OwcaMap values) {
		return vm->execute_code_block(oc, values, nullptr);
	}

	OwcaValue OwcaVM::execute(const OwcaCode &oc, OwcaMap values, OwcaMap *output_dict)
	{
		return vm->execute_code_block(oc, values, output_dict);
	}
	OwcaValue OwcaVM::get_member(OwcaValue self, std::string_view key) {
		return vm->member(self, key);
	}
	void OwcaVM::set_member(OwcaValue self, std::string_view key, OwcaValue value) {
		vm->member(self, key, value);
	}
	OwcaValue OwcaVM::call(OwcaValue func, std::span<OwcaValue> values) {
		return vm->execute_call(func, values);
	}

	OwcaCode OwcaVM::compile(std::string filename, std::string content, std::shared_ptr<NativeCodeProvider> native_code_provider)
	{
		return compile(std::move(filename), std::move(content), {}, std::move(native_code_provider), 1);
	}
	OwcaCode OwcaVM::compile(std::string filename, std::string content, std::vector<std::string> additional_variables, size_t first_line)
	{
		return compile(std::move(filename), std::move(content), std::move(additional_variables), nullptr, first_line);
	}
	OwcaCode OwcaVM::compile(std::string filename, std::string content, std::shared_ptr<NativeCodeProvider> native_code_provider, size_t first_line)
	{
		return compile(std::move(filename), std::move(content), {}, std::move(native_code_provider), first_line);
	}

	OwcaCode OwcaVM::compile(std::string filename, std::string content, std::vector<std::string> additional_variables, std::shared_ptr<NativeCodeProvider> native_code_provider, size_t first_line)
	{
		auto compiler = Internal::AstCompiler{ *vm, std::move(filename), std::move(content), std::move(native_code_provider), first_line };
		auto v = compiler.compile(std::move(additional_variables));
		if (!v)
			throw CompilationFailed{ compiler.filename(), compiler.take_error_messages()};

		return std::move(*v);
	}

	OwcaArray OwcaVM::create_array() const
	{
		return vm->create_array(std::deque<OwcaValue>{});
	}
	OwcaArray OwcaVM::create_array(std::span<OwcaValue> values) const
	{
		return vm->create_array({ values.begin(), values.end() });
	}
	OwcaArray OwcaVM::create_array(std::deque<OwcaValue> values) const
	{
		return vm->create_array(std::move(values));
	}
	OwcaTuple OwcaVM::create_tuple(std::pair<OwcaValue, OwcaValue> values) const
	{
		return vm->create_tuple(values);
	}
	OwcaTuple OwcaVM::create_tuple(std::vector<OwcaValue> values) const
	{
		return vm->create_tuple(std::move(values));
	}
	OwcaMap OwcaVM::create_map() const
	{
		return vm->create_map(std::span<OwcaValue>{});
	}
	OwcaMap OwcaVM::create_map(const std::span<OwcaValue> &values) const
	{
		return vm->create_map(values);
	}
	OwcaMap OwcaVM::create_map(const std::span<std::pair<OwcaValue, OwcaValue>> &values) const
	{
		return vm->create_map(values);
	}
	OwcaMap OwcaVM::create_map(const std::span<std::pair<std::string, OwcaValue>> &values) const
	{
		return vm->create_map(values);
	}
	OwcaSet OwcaVM::create_set(const std::span<OwcaValue> &values) const
	{
		return vm->create_set(values);
	}
	OwcaString OwcaVM::create_string(std::string_view txt) const
	{
		return vm->create_string_from_view(txt);
	}
	void OwcaVM::run_gc() {
		vm->run_gc();
	}
}
