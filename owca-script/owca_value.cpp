#include "stdafx.h"
#include "owca_value.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"
#include "object.h"
#include "array.h"
#include "tuple.h"
#include "dictionary.h"
#include "string.h"
#include "iterator.h"
#include "range.h"
#include "exception.h"

namespace OwcaScript {
	OwcaValue::OwcaValue(OwcaEmpty value): OwcaValue(OwcaValueKind::Empty, nullptr, nullptr) {}
	OwcaValue::OwcaValue(OwcaCompleted value): OwcaValue(OwcaValueKind::Completed, nullptr, nullptr) {}
	OwcaValue::OwcaValue(OwcaRange value): OwcaValue(OwcaValueKind::Range, value.internal_object(), nullptr) {}
	OwcaValue::OwcaValue(OwcaString value): OwcaValue(OwcaValueKind::String, value.internal_value(), nullptr) {}
	OwcaValue::OwcaValue(OwcaFunctions value): OwcaValue(OwcaValueKind::Functions, value.internal_value(), value.internal_self_object()) {}
	OwcaValue::OwcaValue(OwcaMap value): OwcaValue(OwcaValueKind::Map, value.internal_value(), nullptr) {}
	OwcaValue::OwcaValue(OwcaClass value): OwcaValue(OwcaValueKind::Class, value.internal_value(), nullptr) {}
	OwcaValue::OwcaValue(OwcaObject value): OwcaValue(OwcaValueKind::Object, value.internal_value(), nullptr) {}
	OwcaValue::OwcaValue(OwcaTuple value): OwcaValue(OwcaValueKind::Tuple, value.internal_value(), nullptr) {}
	OwcaValue::OwcaValue(OwcaArray value): OwcaValue(OwcaValueKind::Array, value.internal_value(), nullptr) {}
	OwcaValue::OwcaValue(OwcaSet value): OwcaValue(OwcaValueKind::Set, value.internal_value(), nullptr) {}
	OwcaValue::OwcaValue(OwcaException value): OwcaValue(OwcaValueKind::Exception, value.internal_owner(), value.internal_value()) {}	
	OwcaValue::OwcaValue(OwcaIterator value): OwcaValue(OwcaValueKind::Iterator, value.internal_value(), nullptr) {}

	void *OwcaValue::internal_ptr1() const {
		PtrsValue tmp;
		std::memcpy(&tmp, &value_encoded_, sizeof(PtrsValue));
		return (void*)(tmp.ptr1 & ~(std::uintptr_t)15);
	}
	void *OwcaValue::internal_ptr2() const {
		PtrsValue tmp;
		std::memcpy(&tmp, &value_encoded_, sizeof(PtrsValue));
		return tmp.ptr2;
	}

	OwcaValue::OwcaValue(OwcaValueKind kind, void *ptr1, void *ptr2) {
		std::uintptr_t k = (std::uintptr_t)ptr1;
		assert((k & 15) == 0);
		assert((int)kind < 16 && (int)kind >= 0);
		k |= (std::uintptr_t)kind;
		value_encoded_.ptrs.ptr1 = k;
		value_encoded_.ptrs.ptr2 = ptr2;
		assert(this->kind() == kind);
		assert(internal_ptr1() == ptr1);
		assert(internal_ptr2() == ptr2);
	}
	OwcaValue::OwcaValue(OwcaValueKind kind, Number num) {
		assert((int)kind < 16 && (int)kind >= 0);
		value_encoded_.number.value = num;
		value_encoded_.number.kind = (std::uint8_t)kind;
		assert(this->kind() == kind);
	}
	OwcaValueKind OwcaValue::kind() const {
		NumberValue tmp;
		std::memcpy(&tmp, &value_encoded_, sizeof(NumberValue));
		auto k = tmp.kind & 15;
		assert(k < (int)OwcaValueKind::_Count);
		return (OwcaValueKind)k;
	}

	long long int OwcaValue::as_int(OwcaVM vm) const
	{
		if (kind() == OwcaValueKind::Float) {
			auto f = as_float(vm);
			if (std::isnan(f) || std::isinf(f))
				Internal::VM::get(vm).throw_cant_convert_to_integer(f);
			return (long long int)f;
		}
		Internal::VM::get(vm).throw_cant_convert_to_integer(type());
	}

	bool OwcaValue::is_true() const
	{
		return visit(
			[](OwcaEmpty) { return false; },
			[](OwcaCompleted) { return false; },
			[](OwcaRange) { return true; },
			[](Number o) { return o != 0; },
			[](bool o) { return o; },
			[](OwcaString o) { return o.internal_value()->size() != 0; },
			[](OwcaFunctions) { return true; },
			[](OwcaMap o) { return o.size() != 0; },
			[](OwcaClass o) { return true; },
			[&](OwcaObject o) {
				return o.internal_value()->vm->calculate_if_true(*this);
			},
			[](OwcaArray o) { return !o.internal_value()->values.empty(); },
			[](OwcaTuple o) { return !o.internal_value()->values.empty(); },
			[](OwcaSet o) { return o.size() != 0; },
			[](OwcaIterator o) { return !o.completed(); },
			[](OwcaException o) { return true; }
		);
	}

	OwcaEmpty OwcaValue::as_nul(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Empty)
			Internal::VM::get(vm).throw_wrong_type(type(), "Nul");
		return {};
	}
	OwcaCompleted OwcaValue::as_completed(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Completed)
			Internal::VM::get(vm).throw_wrong_type(type(), "Completed");
		return {};
	}
	OwcaRange OwcaValue::as_range(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Range)
			Internal::VM::get(vm).throw_wrong_type(type(), "Range");
		return OwcaRange{ (Internal::Range*)internal_ptr1() };
	}
	bool OwcaValue::as_bool(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Bool)
			Internal::VM::get(vm).throw_wrong_type(type(), "Bool");
		NumberValue tmp;
		std::memcpy(&tmp, &value_encoded_, sizeof(NumberValue));
		return tmp.value != 0;
	}
	Number OwcaValue::as_float(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Float)
			Internal::VM::get(vm).throw_wrong_type(type(), "Float");
		NumberValue tmp;
		std::memcpy(&tmp, &value_encoded_, sizeof(NumberValue));
		return tmp.value;
	}
	OwcaString OwcaValue::as_string(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::String)
			Internal::VM::get(vm).throw_wrong_type(type(), "String");
		return OwcaString{ (Internal::String*)internal_ptr1() };
	}
	OwcaFunctions OwcaValue::as_functions(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Functions)
			Internal::VM::get(vm).throw_wrong_type(type(), "Function");
		return OwcaFunctions{ (Internal::RuntimeFunctions*)internal_ptr1(), (Internal::AllocationBase*)internal_ptr2() };
	}
	OwcaMap OwcaValue::as_map(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Map)
			Internal::VM::get(vm).throw_wrong_type(type(), "Map");
		return OwcaMap{ (Internal::DictionaryShared*)internal_ptr1() };
	}
	OwcaClass OwcaValue::as_class(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Class)
			Internal::VM::get(vm).throw_wrong_type(type(), "Class");
		return OwcaClass{ (Internal::Class*)internal_ptr1() };
	}
	OwcaObject OwcaValue::as_object(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Object)
			Internal::VM::get(vm).throw_wrong_type(type(), "Object");
		return OwcaObject{ (Internal::Object*)internal_ptr1() };
	}
	OwcaArray OwcaValue::as_array(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Array)
			Internal::VM::get(vm).throw_wrong_type(type(), "Array");
		return OwcaArray{ (Internal::Array*)internal_ptr1() };
	}
	OwcaTuple OwcaValue::as_tuple(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Tuple)
			Internal::VM::get(vm).throw_wrong_type(type(), "Tuple");
		return OwcaTuple{ (Internal::Tuple*)internal_ptr1() };
	}
	OwcaSet OwcaValue::as_set(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Set)
			Internal::VM::get(vm).throw_wrong_type(type(), "Set");
		return OwcaSet{ (Internal::DictionaryShared*)internal_ptr1() };
	}
	OwcaException OwcaValue::as_exception(OwcaVM vm) const
	{
		if (kind() == OwcaValueKind::Exception)
			return OwcaException{ (Internal::Object*)internal_ptr1(), (Internal::Exception*)internal_ptr2() };
		if (kind() == OwcaValueKind::Object) {
			auto obj = as_object(vm);
			auto oe = Internal::VM::get(vm).is_exception(obj);
			if (oe != nullptr) {
				return OwcaException{ obj.internal_value(), oe };
			}
		}
		Internal::VM::get(vm).throw_wrong_type(type(), "Exception");
	}
	OwcaIterator OwcaValue::as_iterator(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Iterator)
			Internal::VM::get(vm).throw_wrong_type(type(), "Iterator");
		return OwcaIterator{ (Internal::Iterator*)internal_ptr1() };
	}

	std::string_view OwcaValue::type() const
	{
		return visit(
			[](OwcaEmpty) -> std::string_view { return "Nul"; },
			[](OwcaCompleted) -> std::string_view { return "Completed"; },
			[](OwcaRange) -> std::string_view { return "Range"; },
			[](Number) -> std::string_view { return "Float"; },
			[](bool) -> std::string_view { return "Bool"; },
			[](OwcaString) -> std::string_view { return "String"; },
			[](OwcaFunctions) -> std::string_view { return "Function"; },
			[](OwcaMap) -> std::string_view { return "Map"; },
			[](OwcaClass) -> std::string_view { return "Class"; },
			[](OwcaObject o) -> std::string_view { return o.internal_value()->type(); },
			[](OwcaArray) -> std::string_view { return "Array"; },
			[](OwcaTuple) -> std::string_view { return "Tuple"; },
			[](OwcaSet) -> std::string_view { return "Set"; },
			[](OwcaIterator) -> std::string_view { return "Iterator"; },
			[](OwcaException o) -> std::string_view { return o.internal_owner()->type(); }
			);
	}

	std::string OwcaValue::to_string() const
	{
		return visit(
			[](OwcaEmpty o) -> std::string { return "nul"; },
			[](OwcaCompleted o) -> std::string { return "completed"; },
			[](OwcaRange o) -> std::string { return o.to_string(); },
			[](Number o) -> std::string { return std::format("{}", o); },
			[](bool o) -> std::string { return o ? "true" : "false"; },
			[](OwcaString o) -> std::string { return o.internal_value()->to_string(); },
			[](OwcaFunctions o) -> std::string { return "function-set " + std::string{ o.internal_value()->full_name }; },
			[](OwcaMap o) -> std::string { return o.to_string(); },
			[](OwcaClass o) -> std::string { return o.to_string(); },
			[](OwcaObject o) -> std::string { return o.to_string(); },
			[](OwcaArray o) -> std::string { return o.to_string(); },
			[](OwcaTuple o) -> std::string { return o.to_string(); },
			[](OwcaSet o) -> std::string { return o.to_string(); },
			[](OwcaIterator o) -> std::string { return o.internal_value()->to_string(); },
			[](OwcaException o) -> std::string { return o.internal_value()->to_string(); }
			);
	}

	OwcaValue OwcaValue::call(OwcaVM vm, std::span<OwcaValue> args) const {
		return Internal::VM::get(vm).execute_call(*this, args);
	}

	OwcaValue OwcaValue::member(OwcaVM vm, const std::string& key) const
	{
		return Internal::VM::get(vm).member(*this, key);
	}

	void OwcaValue::member(OwcaVM vm, const std::string& key, OwcaValue val)
	{
		return Internal::VM::get(vm).member(*this, key, std::move(val));
	}

	OwcaValue OwcaValue::call(std::span<OwcaValue> args) const {
		return visit(
			[&](OwcaFunctions o) -> OwcaValue {
				return o.internal_value()->vm->execute_call(*this, args);
			},
			[&](OwcaClass o) -> OwcaValue {
				return o.internal_value()->vm->execute_call(*this, args);
			},
			[&](OwcaObject o) -> OwcaValue {				
				return o.internal_value()->vm->execute_call(*this, args);
			},
			[&](auto) -> OwcaValue { throw std::runtime_error(std::format("Value of type {} is not callable", type())); }
			);
	}

	OwcaValue OwcaValue::member(const std::string& key) const
	{
		return visit(
			[&](OwcaString o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](OwcaFunctions o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](OwcaMap o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](OwcaClass o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](OwcaObject o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](OwcaArray o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](OwcaTuple o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](OwcaSet o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](OwcaIterator o) -> OwcaValue { return o.internal_value()->vm->member(*this, key); },
			[&](auto) -> OwcaValue { throw std::runtime_error(std::format("Value of type {} is not callable", type())); }
			);
	}

	void OwcaValue::member(const std::string& key, OwcaValue val)
	{
		visit(
			[&](OwcaString o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](OwcaFunctions o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](OwcaMap o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](OwcaClass o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](OwcaObject o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](OwcaArray o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](OwcaTuple o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](OwcaSet o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](OwcaIterator o) -> void { o.internal_value()->vm->member(*this, key, val); },
			[&](auto) -> void { throw std::runtime_error(std::format("Value of type {} is not callable", type())); }
			);
	}

	static void gc_mark_value_call(OwcaVM vm, GenerationGC gc, auto o) {
		gc_mark_value(vm, gc, o);
	}

	void gc_mark_value(OwcaVM vm, GenerationGC gc, OwcaValue o) {

		o.visit([&](auto o) -> void {
				gc_mark_value_call(vm, gc, o);
			});
	}
}