#ifndef RC_OWCA_SCRIPT_OWCA_INT_H
#define RC_OWCA_SCRIPT_OWCA_INT_H

#include "stdafx.h"

namespace OwcaScript {
	using OwcaIntInternal = std::int64_t;

	class OwcaInt {
		OwcaIntInternal value;

	public:
		OwcaInt() = default;
		OwcaInt(OwcaIntInternal value) : value(value) {}

		auto internal_value() const { return value; }
		explicit operator OwcaIntInternal() const { return value; }
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
