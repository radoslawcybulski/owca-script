#include "stdafx.h"
#include "owca_value.h"
#include "owca_exception.h"
#include "vm.h"
#include "owca_vm.h"
#include "runtime_function.h"

namespace OwcaScript {
	std::pair<const OwcaInt*, const OwcaFloat*> OwcaValue::get_int_or_float() const
	{
		return visit(
			[](const OwcaInt& o) -> std::pair<const OwcaInt*, const OwcaFloat*> { return { &o, nullptr }; },
			[](const OwcaFloat& o) -> std::pair<const OwcaInt*, const OwcaFloat*> { return { nullptr, &o }; },
			[](const auto&) -> std::pair<const OwcaInt*, const OwcaFloat*> { return { nullptr, nullptr }; }
		);
	}

	OwcaIntInternal OwcaValue::convert_to_int(OwcaVM &vm) const
	{
		auto [i, f] = get_int_or_float();
		if (i) return i->internal_value();
		if (f) {
			auto ii = (OwcaIntInternal)f->internal_value();
			if ((OwcaFloatInternal)ii == f->internal_value()) return ii;
			vm.vm->throw_cant_convert_to_integer(f->internal_value());
		}
		vm.vm->throw_cant_convert_to_integer(type());
		assert(false);
		return {};
	}

	OwcaFloatInternal OwcaValue::convert_to_float(OwcaVM &vm) const
	{
		auto [i, f] = get_int_or_float();
		if (i) return (OwcaFloatInternal)i->internal_value();
		if (f) return f->internal_value();
		vm.vm->throw_cant_convert_to_float(type());
		assert(false);
		return {};
	}

	bool OwcaValue::is_true() const
	{
		return visit(
			[](const OwcaEmpty&) { return false; },
			[](const OwcaRange&) { return true; },
			[](const OwcaInt& o) { return o.internal_value() != 0; },
			[](const OwcaFloat& o) { return o.internal_value() != 0; },
			[](const OwcaBool &o) { return o.internal_value(); },
			[](const OwcaString &o) { return !o.internal_value().empty(); },
			[](const OwcaFunctions&) { return true; }
		);
	}

	OwcaRange OwcaValue::as_range(OwcaVM &vm) const
	{
		if (kind() != OwcaValueKind::Range)
			vm.vm->throw_wrong_type(type(), "range");
		return std::get<OwcaRange>(value_);
	}
	OwcaBool OwcaValue::as_bool(OwcaVM &vm) const
	{
		if (kind() != OwcaValueKind::Bool)
			vm.vm->throw_wrong_type(type(), "bool");
		return std::get<OwcaBool>(value_);
	}
	OwcaInt OwcaValue::as_int(OwcaVM &vm) const
	{
		if (kind() != OwcaValueKind::Int)
			vm.vm->throw_wrong_type(type(), "integer");
		return std::get<OwcaInt>(value_);
	}
	OwcaFloat OwcaValue::as_float(OwcaVM &vm) const
	{
		if (kind() != OwcaValueKind::Float)
			vm.vm->throw_wrong_type(type(), "floating point number");
		return std::get<OwcaFloat>(value_);
	}
	OwcaString OwcaValue::as_string(OwcaVM &vm) const
	{
		if (kind() != OwcaValueKind::String)
			vm.vm->throw_wrong_type(type(), "string");
		return std::get<OwcaString>(value_);
	}
	OwcaFunctions OwcaValue::as_functions(OwcaVM &vm) const
	{
		if (kind() != OwcaValueKind::Functions)
			vm.vm->throw_wrong_type(type(), "function-set");
		return std::get<OwcaFunctions>(value_);
	}
	std::string_view OwcaValue::type() const
	{
		return visit(
			[](const OwcaEmpty&) -> std::string_view { return "nul"; },
			[](const OwcaRange&) -> std::string_view { return "range"; },
			[](const OwcaInt&) -> std::string_view { return "integer"; },
			[](const OwcaFloat&) -> std::string_view { return "floating point number"; },
			[](const OwcaBool&) -> std::string_view { return "bool"; },
			[](const OwcaString&) -> std::string_view { return "string"; },
			[](const OwcaFunctions &o) -> std::string_view { return o.functions->type(); }
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
			[](const OwcaString &o) -> std::string { return std::format("'{}'", o.internal_value()); },
			[](const OwcaFunctions &o) -> std::string { return o.functions->to_string(); }
		);
	}
}