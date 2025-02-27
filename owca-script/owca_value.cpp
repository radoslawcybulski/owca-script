#include "stdafx.h"
#include "owca_value.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"
#include "object.h"
#include "array.h"
#include "dictionary.h"
#include "string.h"

namespace OwcaScript {
	OwcaValue::OwcaValue(OwcaException value) : value_(OwcaObject{ value.internal_owner() }) {}

	std::pair<const OwcaInt*, const OwcaFloat*> OwcaValue::get_int_or_float() const
	{
		return visit(
			[](const OwcaInt& o) -> std::pair<const OwcaInt*, const OwcaFloat*> { return { &o, nullptr }; },
			[](const OwcaFloat& o) -> std::pair<const OwcaInt*, const OwcaFloat*> { return { nullptr, &o }; },
			[](const auto&) -> std::pair<const OwcaInt*, const OwcaFloat*> { return { nullptr, nullptr }; }
		);
	}
	OwcaIntInternal OwcaValue::convert_to_int(OwcaVM vm) const
	{
		auto [i, f] = get_int_or_float();
		if (i) return i->internal_value();
		if (f) {
			auto ii = (OwcaIntInternal)f->internal_value();
			if ((OwcaFloatInternal)ii == f->internal_value()) return ii;
			Internal::VM::get(vm).throw_cant_convert_to_integer(f->internal_value());
		}
		Internal::VM::get(vm).throw_cant_convert_to_integer(type());
	}

	OwcaFloatInternal OwcaValue::convert_to_float(OwcaVM vm) const
	{
		auto [i, f] = get_int_or_float();
		if (i) return (OwcaFloatInternal)i->internal_value();
		if (f) return f->internal_value();
		Internal::VM::get(vm).throw_cant_convert_to_float(type());
	}

	bool OwcaValue::is_true() const
	{
		return visit(
			[](const OwcaEmpty&) { return false; },
			[](const OwcaRange&) { return true; },
			[](const OwcaInt& o) { return o.internal_value() != 0; },
			[](const OwcaFloat& o) { return o.internal_value() != 0; },
			[](const OwcaBool &o) { return o.internal_value(); },
			[](const OwcaString &o) { return o.internal_value()->size() != 0; },
			[](const OwcaFunctions&) { return true; },
			[](const OwcaMap &o) { return o.size() != 0; },
			[](const OwcaClass &o) { return true; },
			[&](const OwcaObject &o) {
				return o.internal_value()->vm->calculate_if_true(*this);
			},
			[](OwcaArray o) { return !o.internal_value()->values.empty(); },
			[](OwcaTuple o) { return !o.internal_value()->values.empty(); },
			[](const OwcaSet &o) { return o.size() != 0; }
		);
	}

	OwcaEmpty OwcaValue::as_nul(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Empty)
			Internal::VM::get(vm).throw_wrong_type(type(), "Nul");
		return {};
	}
	OwcaRange OwcaValue::as_range(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Range)
			Internal::VM::get(vm).throw_wrong_type(type(), "Range");
		return std::get<OwcaRange>(value_);
	}
	OwcaBool OwcaValue::as_bool(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Bool)
			Internal::VM::get(vm).throw_wrong_type(type(), "Bool");
		return std::get<OwcaBool>(value_);
	}
	OwcaInt OwcaValue::as_int(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Int)
			Internal::VM::get(vm).throw_wrong_type(type(), "Integer");
		return std::get<OwcaInt>(value_);
	}
	OwcaFloat OwcaValue::as_float(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Float)
			Internal::VM::get(vm).throw_wrong_type(type(), "Float");
		return std::get<OwcaFloat>(value_);
	}
	const OwcaString &OwcaValue::as_string(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::String)
			Internal::VM::get(vm).throw_wrong_type(type(), "String");
		return std::get<OwcaString>(value_);
	}
	OwcaFunctions OwcaValue::as_functions(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Functions)
			Internal::VM::get(vm).throw_wrong_type(type(), "Function");
		return std::get<OwcaFunctions>(value_);
	}
	OwcaMap OwcaValue::as_map(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Map)
			Internal::VM::get(vm).throw_wrong_type(type(), "Map");
		return std::get<OwcaMap>(value_);
	}
	OwcaClass OwcaValue::as_class(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Class)
			Internal::VM::get(vm).throw_wrong_type(type(), "Class");
		return std::get<OwcaClass>(value_);
	}
	OwcaObject OwcaValue::as_object(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Object)
			Internal::VM::get(vm).throw_wrong_type(type(), "Object");
		return std::get<OwcaObject>(value_);
	}
	OwcaArray OwcaValue::as_array(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Array)
			Internal::VM::get(vm).throw_wrong_type(type(), "Array");
		return std::get<OwcaArray>(value_);
	}
	OwcaTuple OwcaValue::as_tuple(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Tuple)
			Internal::VM::get(vm).throw_wrong_type(type(), "Tuple");
		return std::get<OwcaTuple>(value_);
	}
	OwcaSet OwcaValue::as_set(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Set)
			Internal::VM::get(vm).throw_wrong_type(type(), "Set");
		return std::get<OwcaSet>(value_);
	}
	OwcaException OwcaValue::as_exception(OwcaVM vm) const
	{
		if (kind() != OwcaValueKind::Object)
			Internal::VM::get(vm).throw_wrong_type(type(), "Exception");
		auto oo = as_object(vm);
		auto oe = Internal::VM::get(vm).is_exception(oo);
		if (!oe)
			Internal::VM::get(vm).throw_wrong_type(type(), "Exception");
		return OwcaException{ oo.internal_value(), oe };
	}

	std::string_view OwcaValue::type() const
	{
		return visit(
			[](const OwcaEmpty&) -> std::string_view { return "Nul"; },
			[](const OwcaRange&) -> std::string_view { return "Range"; },
			[](const OwcaInt&) -> std::string_view { return "Integer"; },
			[](const OwcaFloat&) -> std::string_view { return "Float"; },
			[](const OwcaBool&) -> std::string_view { return "Bool"; },
			[](const OwcaString&) -> std::string_view { return "String"; },
			[](const OwcaFunctions &o) -> std::string_view { return "Function"; },
			[](const OwcaMap &o) -> std::string_view { return "Map"; },
			[](const OwcaClass &o) -> std::string_view { return "Class"; },
			[](const OwcaObject &o) -> std::string_view { return o.internal_value()->type(); },
			[](OwcaArray o) -> std::string_view { return "Array"; },
			[](OwcaTuple o) -> std::string_view { return "Tuple"; },
			[](const OwcaSet &o) -> std::string_view { return "Set"; }
			);
	}

	std::string OwcaValue::to_string() const
	{
		return visit(
			[](const OwcaEmpty &o) -> std::string { return "nul"; },
			[](const OwcaRange &o) -> std::string { return std::format("{}..{}", o.lower().internal_value(), o.upper().internal_value()); },
			[](const OwcaInt &o) -> std::string { return std::to_string(o.internal_value()); },
			[](const OwcaFloat &o) -> std::string { return std::to_string(o.internal_value()); },
			[](const OwcaBool &o) -> std::string { return o.internal_value() ? "true" : "false"; },
			[](const OwcaString &o) -> std::string { return o.internal_value()->to_string(); },
			[](const OwcaFunctions &o) -> std::string { return "function-set " + std::string{ o.internal_value()->full_name }; },
			[](const OwcaMap &o) -> std::string { return o.to_string(); },
			[](const OwcaClass &o) -> std::string { return o.to_string(); },
			[](const OwcaObject &o) -> std::string { return o.to_string(); },
			[](OwcaArray o) -> std::string { return o.to_string(); },
			[](OwcaTuple o) -> std::string { return o.to_string(); },
			[](const OwcaSet &o) -> std::string { return o.to_string(); }
			);
	}

	OwcaValue OwcaValue::member(OwcaVM vm, const std::string& key) const
	{
		return Internal::VM::get(vm).member(*this, key);
	}

	void OwcaValue::member(OwcaVM vm, const std::string& key, OwcaValue val)
	{
		return Internal::VM::get(vm).member(*this, key, std::move(val));
	}
}