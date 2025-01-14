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

namespace OwcaScript {
	class OwcaVM;

	enum class OwcaValueKind {
		Empty,
		Range,
		Bool,
		Int,
		Float,
		String,
		Functions,
		Map,
	};

	class OwcaEmpty {};

	class OwcaValue {
		std::variant<OwcaEmpty, OwcaRange, OwcaBool, OwcaInt, OwcaFloat, OwcaString, OwcaFunctions, OwcaMap> value_ = OwcaEmpty{};

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

		OwcaValueKind kind() const { return (OwcaValueKind)value_.index(); }
		std::pair<const OwcaInt*, const OwcaFloat*> get_int_or_float() const;
		OwcaIntInternal convert_to_int(OwcaVM &) const;
		OwcaFloatInternal convert_to_float(OwcaVM &) const;
		bool is_true() const;
		OwcaRange as_range(OwcaVM &) const;
		OwcaBool as_bool(OwcaVM &) const;
		OwcaInt as_int(OwcaVM &) const;
		OwcaFloat as_float(OwcaVM &) const;
		const OwcaString &as_string(OwcaVM &) const;
		OwcaString &as_string(OwcaVM &);
		OwcaFunctions as_functions(OwcaVM &) const;
		OwcaMap &as_map(OwcaVM &);
		const OwcaMap &as_map(OwcaVM &) const;

		std::string_view type() const;
		std::string to_string() const;

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
