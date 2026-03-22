#ifndef RC_OWCA_SCRIPT_OWCA_RANGE_H
#define RC_OWCA_SCRIPT_OWCA_RANGE_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;

	class OwcaRange {
		Number lower_, upper_;

	public:
		OwcaRange(Number lower_, Number upper_) : lower_(lower_), upper_(upper_) {}

		Number lower() const { return lower_; }
		Number upper() const { return upper_; }
		std::pair<Number, Number> internal_values() const;
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaRange>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaRange v, FormatContext& ctx) const
		{
			return format_to(ctx.out(), "{}:{}", v.lower(), v.upper());  
		}
		template<class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx)
		{
			return ctx.begin();
		}
	};
}

#endif
