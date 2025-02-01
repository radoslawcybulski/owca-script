#ifndef RC_OWCA_SCRIPT_OWCA_BOOL_H
#define RC_OWCA_SCRIPT_OWCA_BOOL_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaBool {
		bool value;

	public:
		explicit OwcaBool(bool value) : value(value) {}

		auto internal_value() const { return value; }
		explicit operator bool() const { return value; }
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaBool>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaBool v, FormatContext& ctx) const
		{
			return format_to(ctx.out(), "{}", v.internal_value() ? "true" : "false");  
		}
		template<class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx)
		{
			return ctx.begin();
		}
	};
}

#endif
