#include "stdafx.h"
#include "owca_class.h"
#include "object.h"
#include "owca_value.h"
#include "vm.h"

namespace OwcaScript {
	void NativeClassInterface::get_member(OwcaVM vm, std::string_view, std::span<char> native_storage, OwcaValue &) {
		Internal::VM::get(vm).throw_not_implemented(std::format("get_member for class {} not implemented on native class", Internal::VM::get(vm).current_class_in_progress));
	}
	void NativeClassInterface::set_member(OwcaVM vm, std::string_view, std::span<char> native_storage, const OwcaValue &) {
		Internal::VM::get(vm).throw_not_implemented(std::format("set_member for class {} not implemented on native class", Internal::VM::get(vm).current_class_in_progress));
	}

	std::string OwcaClass::to_string() const
	{
		return object->to_string();
	}

	OwcaValue OwcaClass::operator [] (const std::string &key) const
	{
		return object->vm->member(*this, key);
	}

	bool OwcaClass::has_base_class(OwcaClass base) const
	{
		auto it = object->all_base_classes.find(base.internal_value());
		return it != object->all_base_classes.end();
	}
}