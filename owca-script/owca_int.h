#ifndef RC_OWCA_SCRIPT_OWCA_INT_H
#define RC_OWCA_SCRIPT_OWCA_INT_H

#include "stdafx.h"

namespace OwcaScript {
	using OwcaIntInternal = std::int64_t;
	class OwcaVM;

	class OwcaInt {
		OwcaIntInternal value;

		static void throw_error(OwcaVM vm, std::string_view msg);
	public:
		OwcaInt() = default;
		OwcaInt(OwcaIntInternal value) : value(value) {}
		template <std::integral T> OwcaInt(OwcaVM vm, T v, std::string_view name) {
			auto v2 = (OwcaIntInternal)v;
			if (v2 != v)
				throw_error(vm, std::format("{} {} is out of range of allowed values ({} -> {}) for used integer type", name, v, std::numeric_limits<OwcaIntInternal>::min(), std::numeric_limits<OwcaIntInternal>::max()));
			value = v2;
		}

		auto internal_value() const { return value; }
		explicit operator OwcaIntInternal() const { return value; }

		template <std::integral T> T as(OwcaVM vm, std::string_view name, std::optional<T> min = std::nullopt, std::optional<T> max = std::nullopt) {
			if (!min) min = std::numeric_limits<T>::min();
			if (!max) max = std::numeric_limits<T>::max();
			auto v = (T)value;
			if (v != value || v < *min || v > *max)
			throw_error(vm, std::format("{} {} is out of range of allowed values ({} -> {}) for used integer type", name, value, *min, *max));
			return v;
		}
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaInt>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaInt v, FormatContext& ctx) const
		{
			return format_to(ctx.out(), "{}", v.internal_value());  
		}
		template<class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx)
		{
			return ctx.begin();
		}
	};
}

#endif
