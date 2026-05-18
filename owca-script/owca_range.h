#ifndef RC_OWCA_SCRIPT_OWCA_RANGE_H
#define RC_OWCA_SCRIPT_OWCA_RANGE_H

#include "stdafx.h"

namespace OwcaScript {
	class OwcaValue;
	namespace Internal {
		struct Range;
	}

	class OwcaRange {
		Internal::Range *object;

	public:
		OwcaRange(Internal::Range *object) : object(object) {}

		auto internal_object() const { return object; }

		std::string to_string() const;
		Number lower() const;
		Number upper() const;
		Number step() const;
		Number size() const;
		bool is(OwcaRange other) const { return object == other.object; }

		bool operator == (OwcaRange other) const {
			return lower() == other.lower() && upper() == other.upper() && step() == other.step();
		}
		bool operator != (OwcaRange other) const { return !(*this == other); }

		friend void gc_mark_value(const OwcaVM &vm, GenerationGC gc, OwcaRange);
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaRange>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaRange v, FormatContext& ctx) const
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
