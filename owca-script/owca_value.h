#ifndef RC_OWCA_SCRIPT_OWCA_VALUE_H
#define RC_OWCA_SCRIPT_OWCA_VALUE_H

#include "stdafx.h"
#include "owca_bool.h"
#include "owca_int.h"
#include "owca_float.h"
#include "owca_string.h"
#include "owca_functions.h"
#include "owca_range.h"
#include "owca_map.h"
#include "owca_class.h"
#include "owca_object.h"
#include "owca_tuple.h"
#include "owca_array.h"

namespace OwcaScript {
	class OwcaVM;

	namespace Internal {
		class VM;
	}

	enum class OwcaValueKind {
		Empty,
		Range,
		Bool,
		Int,
		Float,
		String,
		Functions,
		Map,
		Class,
		Object,
		Tuple,
		Array,
	};

	class OwcaEmpty {};

	class OwcaValue {
		friend class Internal::VM;

		std::variant<OwcaEmpty, OwcaRange, OwcaBool, OwcaInt, OwcaFloat, OwcaString, OwcaFunctions, OwcaMap, OwcaClass, OwcaObject, OwcaTuple, OwcaArray> value_ = OwcaEmpty{};

	public:
		OwcaValue() : value_(OwcaEmpty{}) {}
		OwcaValue(OwcaEmpty value) : value_(value) {}
		OwcaValue(OwcaRange value) : value_(value) {}
		OwcaValue(OwcaBool value) : value_(value) {}
		OwcaValue(OwcaInt value) : value_(value) {}
		OwcaValue(OwcaFloat value) : value_(value) {}
		OwcaValue(OwcaString value) : value_(std::move(value)) {}
		OwcaValue(OwcaFunctions value) : value_(std::move(value)) {}
		OwcaValue(OwcaMap value) : value_(std::move(value)) {}
		OwcaValue(OwcaClass value) : value_(std::move(value)) {}
		OwcaValue(OwcaObject value) : value_(std::move(value)) {}
		OwcaValue(OwcaTuple value) : value_(std::move(value)) {}
		OwcaValue(OwcaArray value) : value_(std::move(value)) {}

		OwcaValueKind kind() const { return (OwcaValueKind)value_.index(); }
		std::pair<const OwcaInt*, const OwcaFloat*> get_int_or_float() const;
		OwcaIntInternal convert_to_int(OwcaVM &) const;
		OwcaFloatInternal convert_to_float(OwcaVM &) const;
		bool is_true() const;

		OwcaEmpty as_nul(OwcaVM &) const;
		OwcaRange as_range(OwcaVM &) const;
		OwcaBool as_bool(OwcaVM &) const;
		OwcaInt as_int(OwcaVM &) const;
		OwcaFloat as_float(OwcaVM &) const;
		const OwcaString &as_string(OwcaVM &) const;
		OwcaFunctions as_functions(OwcaVM &) const;
		OwcaMap as_map(OwcaVM &) const;
		OwcaClass as_class(OwcaVM &) const;
		OwcaObject as_object(OwcaVM &) const;
		OwcaTuple as_tuple(OwcaVM &) const;
		OwcaArray as_array(OwcaVM &) const;

		std::string_view type() const;
		std::string to_string() const;

		OwcaValue member(OwcaVM& vm, const std::string& key) const;
		void member(OwcaVM& vm, const std::string& key, OwcaValue val);

		template <typename ... F> auto visit(F &&...fns) const {
			struct overloaded : F... {
				using F::operator()...;
			};
			return std::visit(overloaded{std::forward<F>(fns)...}, value_);
		}
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaValue>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaValue v, FormatContext& ctx) const
		{
			return format_to(ctx.out(), "{}", v.to_string());  
		}
		template<class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx)
		{
			return ctx.begin();
		}
	};
}

#endif
