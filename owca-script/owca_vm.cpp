#include "stdafx.h"
#include "owca_vm.h"
#include "vm.h"
#include "owca_code.h"
#include "ast_compiler.h"

namespace OwcaScript {
	static thread_local Internal::VM *current_vm_ptr = nullptr;

	void Internal::set_current_vm(Internal::VM *vm) {
		current_vm_ptr = vm;
	}

	Internal::VM &Internal::current_vm() {
#ifdef DEBUG
		if (!current_vm_ptr) [[unlikely]] {
			throw std::runtime_error("no VM is active in this thread");
		}
#endif
		return *current_vm_ptr;
	}
	OwcaVM::CompilationFailed::CompilationFailed(std::string filename_, std::vector<OwcaErrorMessage> error_messages_) : filename_(std::move(filename_)), error_messages_(std::move(error_messages_)) {
		err_msg = "compilation of file `" + this->filename_ + "` failed:";
		for(auto &m : this->error_messages_) {
			err_msg += "\n";
			err_msg += m.to_string();
		}
	}

	OwcaVM::OwcaVM() : vm(std::make_unique<Internal::VM>())
	{
	}

	OwcaVM::~OwcaVM() {
		deactivate();
	}

	void OwcaVM::activate() {
		if (current_vm_ptr) {
			throw std::runtime_error("another VM is already active in this thread");
		}
		Internal::set_current_vm(vm.get());
	}

	void OwcaVM::deactivate() {
		if (current_vm_ptr != vm.get()) {
			throw std::runtime_error("this VM is not active in this thread");
		}
		Internal::set_current_vm(nullptr);
	}

	IdentifierIndex OwcaVM::get_identifier_index(std::string_view name) {
		return Internal::current_vm().get_identifier_index(name);
	}
	std::string_view OwcaVM::get_identifier_name(IdentifierIndex index) {
		return Internal::current_vm().get_identifier_name(index);
	}

	OwcaNamespace OwcaVM::execute(const OwcaCodeBuffer &oc, std::shared_ptr<NativeCodeProvider> native_code_provider) {
		assert(current_vm_ptr == vm.get());
		return vm->execute_code_block(oc, std::move(native_code_provider));
	}
	OwcaValue OwcaVM::get_member(OwcaValue self, std::string_view key) {
		assert(current_vm_ptr == vm.get());
		return vm->member(self, key);
	}
	void OwcaVM::set_member(OwcaValue self, std::string_view key, OwcaValue value) {
		assert(current_vm_ptr == vm.get());
		vm->member(self, key, value);
	}
	OwcaValue OwcaVM::call(OwcaValue func, std::span<OwcaValue> values) {
		assert(current_vm_ptr == vm.get());
		return vm->execute_call(func, values);
	}

	OwcaCodeBuffer OwcaVM::compile(std::string filename, std::string content, size_t first_line)
	{
	    assert(current_vm_ptr == vm.get());
		return vm->compile(std::move(filename), std::move(content), first_line);
	}

	OwcaArray OwcaVM::create_array() const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_array(std::deque<OwcaValue>{});
	}
	OwcaArray OwcaVM::create_array(std::span<OwcaValue> values) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_array({ values.begin(), values.end() });
	}
	OwcaArray OwcaVM::create_array(std::deque<OwcaValue> values) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_array(std::move(values));
	}
	OwcaTuple OwcaVM::create_tuple(std::pair<OwcaValue, OwcaValue> values) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_tuple(values);
	}
	OwcaTuple OwcaVM::create_tuple(std::vector<OwcaValue> values) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_tuple(std::move(values));
	}
	OwcaMap OwcaVM::create_map() const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_map(std::span<OwcaValue>{});
	}
	OwcaMap OwcaVM::create_map(const std::span<OwcaValue> &values) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_map(values);
	}
	OwcaMap OwcaVM::create_map(const std::span<std::pair<OwcaValue, OwcaValue>> &values) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_map(values);
	}
	OwcaMap OwcaVM::create_map(const std::span<std::pair<std::string, OwcaValue>> &values) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_map(values);
	}
	OwcaSet OwcaVM::create_set(const std::span<OwcaValue> &values) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_set(values);
	}
	OwcaString OwcaVM::create_string(std::string_view txt) const
	{
		assert(current_vm_ptr == vm.get());
		return vm->create_string_from_view(txt);
	}
	void OwcaVM::run_gc() {
		assert(current_vm_ptr == vm.get());
		vm->run_gc();
	}
}
