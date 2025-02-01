#ifndef RC_OWCA_SCRIPT_OWCA_FLOAT_H
#define RC_OWCA_SCRIPT_OWCA_FLOAT_H

#include "stdafx.h"

namespace OwcaScript {
	using OwcaFloatInternal = double;

	class OwcaFloat {
		OwcaFloatInternal value;

	public:
		OwcaFloat() = default;
		OwcaFloat(OwcaFloatInternal value) : value(value) {}

		auto internal_value() const { return value; }
		explicit operator OwcaFloatInternal() const { return value; }
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaFloat>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaFloat v, FormatContext& ctx) const
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
