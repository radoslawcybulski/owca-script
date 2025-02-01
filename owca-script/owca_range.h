#ifndef RC_OWCA_SCRIPT_OWCA_RANGE_H
#define RC_OWCA_SCRIPT_OWCA_RANGE_H

#include "stdafx.h"
#include "owca_int.h"

namespace OwcaScript {
	class OwcaRange {
		OwcaInt lower_, upper_;

	public:
		OwcaRange(OwcaInt lower_, OwcaInt upper_) : lower_(lower_), upper_(upper_) {}

		auto lower() const { return lower_; }
		auto upper() const { return upper_; }
		std::pair<OwcaIntInternal, OwcaIntInternal> internal_values() const {
			return { lower_.internal_value(), upper_.internal_value() };
		}
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
