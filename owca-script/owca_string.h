#ifndef RC_OWCA_SCRIPT_OWCA_STRING_H
#define RC_OWCA_SCRIPT_OWCA_STRING_H

#include "stdafx.h"

namespace OwcaScript {
	using OwcaStringInternal = std::string;

	class OwcaString {
		OwcaStringInternal value;

	public:
		OwcaString() = default;
		OwcaString(OwcaStringInternal value) : value(std::move(value)) {}

		const auto &internal_value() const { return value; }
		explicit operator std::string_view() const { return value; }
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaString>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaString v, FormatContext& ctx) const
		{
			return format_to(ctx.out(), "{}", v.internal_value);  
		}
		template<class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx)
		{
			return ctx.begin();
		}
	};
}

#endif
