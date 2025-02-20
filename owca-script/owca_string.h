#ifndef RC_OWCA_SCRIPT_OWCA_STRING_H
#define RC_OWCA_SCRIPT_OWCA_STRING_H

#include "stdafx.h"

namespace OwcaScript {
	namespace Internal {
		struct String;
	}

	class OwcaValue;
	
	class OwcaString {
		Internal::String *str;

	public:
		OwcaString(Internal::String *str) : str(str) {}

		auto internal_value() const { return str; }
		std::string_view text() const;
		explicit operator std::string_view() const { return text(); }

		OwcaValue substr(size_t start, size_t end) const;
		OwcaValue operator [] (size_t pos) const;
		size_t size() const;
		size_t hash() const;
	};
}

namespace std {
	template <>
	struct formatter<OwcaScript::OwcaString>
	{
		template <typename FormatContext>
		auto format(OwcaScript::OwcaString v, FormatContext& ctx) const
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
