#include "owca-script/owca_iterator.h"
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

	long long int OwcaValue::as_int() const
	{
		if (auto f = as_float_maybe()) {
			if (std::isnan(*f) || std::isinf(*f))
				Internal::current_vm().throw_cant_convert_to_integer(*f);
			return (long long int)*f;
		}
		Internal::current_vm().throw_cant_convert_to_integer(type());
	}

	bool OwcaValue::is_true() const
	{
		return Internal::current_vm().calculate_if_true(*this);
	}

	[[noreturn]] void OwcaValue::throw_wrong_type(std::string_view expected) const {
		Internal::current_vm().throw_wrong_type(type(), expected);
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
			[](OwcaException o) -> std::string_view { return o.internal_owner()->type(); },
			[](OwcaNamespace) -> std::string_view { return "Namespace"; }
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
			[](OwcaException o) -> std::string { return o.internal_value()->to_string(); },
			[](OwcaNamespace o) -> std::string { return o.to_string(); }
			);
	}

	OwcaValue OwcaValue::call(std::span<OwcaValue> args) const {
		return Internal::current_vm().execute_call(*this, args);
	}

	OwcaValue OwcaValue::member(const std::string& key) const
	{
		return Internal::current_vm().member(*this, key);
	}

	void OwcaValue::member(const std::string& key, OwcaValue val)
	{
		return Internal::current_vm().member(*this, key, std::move(val));
	}

	OwcaValue OwcaValue::call_with_args(std::span<OwcaValue> args) const {
		return Internal::current_vm().execute_call(*this, args);
	}

	std::optional<OwcaException> OwcaValue::as_exception_maybe() const
	{
		if (auto v = as_object_maybe()) {
			if (auto o =  v->user_data_maybe<Internal::Exception>()) {
				return OwcaException{ v->internal_value(), o };
			}
		}
		return std::nullopt;
	}

	static void gc_mark_value_call(GenerationGC gc, auto o) {
		gc_mark_value(gc, o);
	}

	void gc_mark_value(GenerationGC gc, OwcaValue o) {
		o.visit([&](auto o) -> void {
				gc_mark_value_call(gc, o);
			});
	}

	namespace Internal {
		void throw_cant_convert_to_number(size_t I, OwcaValue v) {
			Internal::current_vm().throw_cant_convert_to_float_message(std::format("{} argument ({}) can't be converted to a number value", I + 1, v.type()));
		}

		OwcaIterator convert_impl2(size_t I, OwcaIterator *b, OwcaValue v) {
			if (auto q = v.as_iterator_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not an iterator", I + 1, v.type()));
		}
		bool convert_impl2(size_t I, bool *b, OwcaValue v) {
			if (auto q = v.as_bool_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) can't be converted to bool", I + 1, v.type()));
		}
		std::string convert_impl2(size_t I, std::string *b, OwcaValue v) {
			if (auto q = v.as_string_maybe()) {
				return std::string{ q->text() };
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a string", I + 1, v.type()));
		}
		std::string_view convert_impl2(size_t I, std::string_view *b, OwcaValue v) {
			if (auto q = v.as_string_maybe()) {
				return q->text();
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a string", I + 1, v.type()));
		}
		OwcaEmpty convert_impl2(size_t I, OwcaEmpty *b, OwcaValue v) {
			if (auto q = v.as_nul_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a nul value", I + 1, v.type()));
		}
		OwcaRange convert_impl2(size_t I, OwcaRange *b, OwcaValue v) {
			if (auto q = v.as_range_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a range", I + 1, v.type()));
		}
		Number convert_impl2(size_t I, Number *b, OwcaValue v) {
			if (auto q = v.as_float_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a floating point value", I + 1, v.type()));
		}
		OwcaString convert_impl2(size_t I, OwcaString *b, OwcaValue v) {
			if (auto q = v.as_string_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a string", I + 1, v.type()));
		}
		OwcaFunctions convert_impl2(size_t I, OwcaFunctions *b, OwcaValue v) {
			if (auto q = v.as_functions_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a function set", I + 1, v.type()));
		}
		OwcaMap convert_impl2(size_t I, OwcaMap *b, OwcaValue v) {
			if (auto q = v.as_map_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a dictionary", I + 1, v.type()));
		}
		OwcaClass convert_impl2(size_t I, OwcaClass *b, OwcaValue v) {
			if (auto q = v.as_class_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a type", I + 1, v.type()));
		}
		OwcaObject convert_impl2(size_t I, OwcaObject *b, OwcaValue v) {
			if (auto q = v.as_object_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not an object", I + 1, v.type()));
		}
		OwcaArray convert_impl2(size_t I, OwcaArray *b, OwcaValue v) {
			if (auto q = v.as_array_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not an array", I + 1, v.type()));
		}
		OwcaTuple convert_impl2(size_t I, OwcaTuple *b, OwcaValue v) {
			if (auto q = v.as_tuple_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a tuple", I + 1, v.type()));
		}
		OwcaSet convert_impl2(size_t I, OwcaSet *b, OwcaValue v) {
			if (auto q = v.as_set_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not a set", I + 1, v.type()));
		}
		OwcaException convert_impl2(size_t I, OwcaException *b, OwcaValue v) {
			if (auto q = v.as_exception_maybe()) {
				return *q;
			}
			Internal::current_vm().throw_cant_call(std::format("{} argument ({}) is not an exception object", I + 1, v.type()));
		}
		OwcaValue convert_impl2(size_t I, OwcaValue *b, OwcaValue v) {
			return v;
		}
	}
}